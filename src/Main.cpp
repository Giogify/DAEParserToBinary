#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <thread>
#include <queue>
#include <iomanip>

#include "pugixml-1.12/src/pugixml.hpp"

//#include "/Users/giavo/source/repos/DAEParser/inc/DirectXTex/DirectXTex/DirectXTex.h"
//#include "/Users/giavo/source/repos/DAEParser/inc/DirectXTex/DirectXTex/DirectXTex.inl"

using namespace std;
using FS = ifstream;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

#define STRNUM "0123456789.-e"

namespace Collada {

	inline void ers(string& _str, const string _upto) {
		_str.erase(0u, _str.find(_upto));
	}
	inline string prsstr(string& _str, const string _upto) {
		ers(_str, _upto);
		size_t off0 = _str.find('\"');
		size_t off1 = _str.find('\"', ++off0) - off0;
		string sub = _str.substr(off0, off1);
		_str.erase(0u, off0 + off1 + 1);
		return sub;
	}
	inline string prsbtw(string& _str, const string _btw, const string _upto) {
		ers(_str, _upto);
		size_t off0 = _str.find(_btw);
		size_t off1 = _str.find(_btw, ++off0) - off0;
		string sub = _str.substr(off0, off1);
		_str.erase(0u, off0 + off1 + 1);
		return sub;
	}
	inline string prsbtw2(string& _str, const string _l, const string _r, const string _upto) {
		ers(_str, _upto);
		size_t off0 = _str.find(_l);
		size_t off1 = _str.find(_r, ++off0) - off0;
		string sub = _str.substr(off0, off1);
		_str.erase(0u, off0 + off1 + 1);
		return sub;
	}
	inline string gensub(string& _str, string _what) {
		ers(_str, _what);
		_what.insert(1u, 1u, '/');
		string s = _str.substr(0u, _str.find(_what) + _what.size());
		_str.erase(0u, _str.find(_what) + _what.size());
		return s;
	}
	inline string gensub2(string& _str, string _what) {
		ers(_str, _what);
		_what.insert(1u, 1u, '/');
		_what.erase(0u, 1u);
		_what.insert(_what.size(), 1u, '>');
		string s = _str.substr(0u, _str.find(_what) + _what.size());
		_str.erase(0u, _str.find(_what) + _what.size());
		return s;
	}
	inline string gensub3(string& _str, string _what) {
		ers(_str, _what);
		string s = _str.substr(0u, _str.find("/>") + 2u);
		_str.erase(0u, _str.find("/>") + 2u);
		return s;
	}
	inline size_t srchstr(const string& _s,const string _wh){
		const char* st=_s.c_str();
		u32 ct{};
		while(*st!=0){
			bool mt=true;
			for(u32 i=0; i<_wh.size(); i++){
				if(_wh[i]!=*(st+i)){
					mt=false;
					break;
				}
			}
			if(mt) ct++;
			st++;
		}
		return ct;

		/*u32 sz = _wh.size();
		vector<char> vch(sz);
		for (u32 i = 0; i < sz; i++) vch[i] = _wh[i];
		deque<char> qch(sz);
		const char* st = _s.c_str();
		u32 ct{};
		while (*st != 0) {
		qch.pop_front();
		qch.push_back(*st++);
		bool mt = true;
		for (u32 i = 0; i < sz; i++) if (vch[i] != qch[i]) { mt = false; break; }
		if (mt) ct++;
		}
		return ct;*/

		/*u64 ct{}, off{};
		while (1) {
		off = _s.find(_wh, off == 0 ? off : off + 1);
		if (off == string::npos) return ct;
		ct++;
		}*/
	}
	inline u64 ctnode(const pugi::xml_node& node, const string what){
		u64 ct{};
		for(auto it=node.begin(); it!=node.end(); it++){
			auto& nd=*it;
			if(nd.name()==what) ct++;
		}
		return ct;
	}

	typedef struct FLOAT2 {
		float x{};
		float y{};
	} F2;
	typedef struct FLOAT3 {
		float x{};
		float y{};
		float z{};
	} F3;
	typedef struct FLOAT4 {
		float x{};
		float y{};
		float z{};
		float w{};
	} F4;
	typedef struct COLOR4 {
		float r{};
		float g{};
		float b{};
		float a{};
	} C4;
	typedef struct MATRIX {
		float
			e11{}, e12{}, e13{}, e14{},
			e21{}, e22{}, e23{}, e24{},
			e31{}, e32{}, e33{}, e34{},
			e41{}, e42{}, e43{}, e44{};
	} MTX;

	C4 prscol(string& _s) {
		C4 color{};
		_s.erase(0u, _s.find_first_of(STRNUM));
		color.r = stof(_s.substr(0u, _s.find_first_not_of(STRNUM)));
		_s.erase(0u, _s.find_first_not_of(STRNUM) + 1);
		color.g = stof(_s.substr(0u, _s.find_first_not_of(STRNUM)));
		_s.erase(0u, _s.find_first_not_of(STRNUM) + 1);
		color.b = stof(_s.substr(0u, _s.find_first_not_of(STRNUM)));
		_s.erase(0u, _s.find_first_not_of(STRNUM) + 1);
		color.a = stof(_s.substr(0u, _s.find_first_not_of(STRNUM)));
		_s.erase(0u, _s.find_first_not_of(STRNUM) + 1);
		return color;
	}

	struct Unit {
		string name{};
		double metereqv{};

		void prs(const pugi::xml_node& node) {
			name=node.attribute("name").as_string();
			metereqv=node.attribute("meter").as_double();
		}
		u64 size() {
			u64 sz{};
			sz += name.size();
			sz += sizeof(double);
			return sz;
		}
	};
	typedef struct Contributor {
		string author{};
		string auth_tool{};

		void prs(const pugi::xml_node& node) {
			author=node.child_value("author");
			auth_tool=node.child_value("authoring_tool");
		}
		u64 size() {
			u64 sz{};
			sz += author.size();
			sz += auth_tool.size();
			return sz;
		}
	} CTB;
	typedef struct Asset {
		CTB ctb{};
		string tmcre{};
		string tmmod{};
		Unit unit{};
		string up_axis{};

		void prs(const pugi::xml_node& node) {
			ctb.prs(node.child("contributor"));
			tmcre=node.child_value("created");
			tmmod=node.child_value("modified");
			unit.prs(node.child("unit"));
			up_axis=node.child_value("up_axis");
			
			/*ctb.prs(gensub(_s, "<contributor>"));
			tmcre = prsbtw2(_s, ">", "<", "created");
			tmmod = prsbtw2(_s, ">", "<", "modified");
			ers(_s, "<unit");
			string sub = _s.substr(0u, _s.find("/>") + 2);
			unit.prs(sub);
			_s.erase(0u, _s.find("/>") + 2);
			up_axis = prsbtw2(_s, ">", "<", "up_axis");*/
		}
		u64 size() {
			u64 sz{};
			sz += ctb.size();
			sz += tmcre.size();
			sz += tmmod.size();
			sz += unit.size();
			sz += up_axis.size();
			return sz;
		}
	} AST;
	struct Color {
		string sid{};
		C4 color{};

		void prs(string _s) {
			sid = prsstr(_s, "sid");
			color = prscol(_s);
		}
	};
	struct Surface {
		string type{};
		string img{};
		void prs(string _s) {
			type = prsstr(_s, "type");
			img = prsbtw2(_s, ">", "<", "init_from");
		}
		u64 size() {
			u64 sz{};
			sz += type.size();
			sz += img.size();
			return sz;
		}
	};
	typedef struct Sampler2D {
		string source{};

		void prs(string _s) {
			source = prsbtw2(_s, ">", "<", "source");
		}
		u64 size() {
			u64 sz{};
			sz += source.size();
			return sz;
		}
	} SMP2D;
	typedef struct EffectTechniqueLambertEmission {
		Color color{};

		void prs(string _s) {
			color.prs(gensub2(_s, "<color"));
		}
		u64 size() {
			u64 sz{};
			sz += sizeof Color;
			return sz;
		}
	} FXTLEmission;
	typedef struct EffectTechniqueLambertDiffuse {
		string texture{};
		string texcoord{};

		void prs(string _s) {
			texture = prsstr(_s, "texture");
			texcoord = prsstr(_s, "texcoord");
		}
		u64 size() {
			u64 sz{};
			sz += texture.size();
			sz += texcoord.size();
			return sz;
		}
	} FXTLDiffuse;
	typedef struct EffectTechniqueLambertTransparent {
		string opaque{};
		Color color{};

		void prs(string _s) {
			opaque = prsstr(_s, "opaque");
			color.prs(gensub2(_s, "<color"));
		}
		u64 size() {
			u64 sz{};
			sz += opaque.size();
			sz += sizeof Color;
			return sz;
		}
	} FXTLTransparent;
	typedef struct EffectTechniqueLambertIOR {
		string sid{};
		float ior{};

		void prs(string _s) {
			sid = prsstr(_s, "sid");
			ior = stof(_s.substr(_s.find_first_of(STRNUM), _s.find_first_not_of(STRNUM, _s.find_first_of(STRNUM))));
		}
		u64 size() {
			u64 sz{};
			sz += sid.size();
			sz += sizeof(float);
			return sz;
		}
	} FXTLIOR;
	typedef struct EffectTechniqueLambert {
		FXTLEmission e{};
		FXTLDiffuse d{};
		FXTLTransparent tp{};
		FXTLIOR ior{};

		void prs(string _s) {
			e.prs(gensub(_s, "<emission>"));
			d.prs(gensub(_s, "<diffuse>"));
			tp.prs(gensub2(_s, "<transparent"));
			ior.prs(gensub(_s, "<index_of_refraction>"));
		}
		u64 size() {
			u64 sz{};
			sz += e.size();
			sz += d.size();
			sz += tp.size();
			sz += ior.size();
			return sz;
		}
	} FXTL;
	typedef struct EffectTechnique {
		string sid{};
		FXTL lb{};

		void prs(string _s) {
			sid = prsstr(_s, "sid");
			lb.prs(gensub(_s, "<lambert>"));
		}
		u64 size() {
			u64 sz{};
			sz += sid.size();
			sz += lb.size();
			return sz;
		}
	} FXTNQ;
	typedef struct NewParameter {
		string sid{};
		Surface surface{};
		SMP2D sampler2d{};

		void prs(string _s) {
			sid = prsstr(_s, "sid");
			if (srchstr(_s, "<surface ") != 0) surface.prs(gensub2(_s, "<surface"));
			if (srchstr(_s, "<sampler2D>") != 0) sampler2d.prs(gensub(_s, "<sampler2D>"));
		}
		u64 size() {
			u64 sz{};
			sz += sid.size();
			sz += surface.size();
			sz += sampler2d.size();
			return sz;
		}
	} NewParam;
	typedef struct Effect {
		string id{};
		vector<NewParam> nps{};
		FXTNQ technique{};

		void prs(const pugi::xml_node& node) {
			id=node.attribute("id").as_string();
			const auto& profile_COMMON=node.first_child();
			
			nps.resize(ctnode(node, "newparam"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			for (auto& e : nps) sz += e.size();
			sz += technique.size();
			return sz;
		}
	} FX;
	typedef struct LibraryEffect {
		vector<FX> fx{};

		void prs(const pugi::xml_node& node) {
			fx.resize(distance(node.begin(),node.end()));
			for(auto it=node.begin(); it!=node.end(); it++) fx[distance(node.begin(),it)].prs(*it);
		}
		u64 size() {
			u64 sz{};
			for (auto& e : fx) sz += e.size();
			return sz;
		}
	} LibFX;
	typedef struct Image {
		string id{};
		string name{};
		string img{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");
			string sub = prsbtw2(_s, ">", "<", "init_from");
			sub.erase(sub.find(".jpg"), sub.size()).append(".jpg");
			img = sub;
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			sz += img.size();
			return sz;
		}
	} IMG;
	typedef struct LibraryImage {
		vector<IMG> images{};

		void prs(string _s) {
			images.resize(srchstr(_s, "<image"));
			for (auto& e : images) e.prs(gensub2(_s, "<image"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : images) sz += e.size();
			return sz;
		}
	} LibIMG;
	typedef struct InstanceEffect {
		string src{};

		void prs(string _s) {
			src = prsstr(_s, "url").erase(0u, 1u);
		}
		u64 size() {
			return src.size();
		}
	} IE;
	typedef struct Material {
		string id{};
		string name{};
		IE ie{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");
			ie.prs(_s);
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			sz += ie.size();
			return sz;
		}
	} MTL;
	typedef struct LibraryMaterial {
		vector<MTL> mats{};

		void prs(string _s) {
			mats.resize(srchstr(_s, "<material"));
			for (auto& e : mats) e.prs(gensub2(_s, "<material"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : mats) sz += e.size();
			return sz;
		}
	} LibMTL;
	typedef struct Parameter {
		string name{};
		string type{};

		void prs(string _s) {
			name = prsstr(_s, "name");
			type = prsstr(_s, "type");
		}
		u64 size() {
			u64 sz{};
			sz += name.size();
			sz += type.size();
			return sz;
		}
	} Param;
	typedef struct Accessor {
		string source{};
		u64 count{};
		u64 stride{};
		vector<Param> params{};

		void prs(string _s) {
			source = prsstr(_s, "source").erase(0u, 1u);
			count = stoi(prsstr(_s, "count"));
			stride = stoi(prsstr(_s, "stride"));
			params.resize(srchstr(_s, "<param"));
			for (auto& e : params) e.prs(gensub3(_s, "<param"));
		}
		u64 size() {
			u64 sz{};
			sz += source.size();
			sz += sizeof u64;
			sz += sizeof u64;
			for (auto& e : params) sz += e.size();
			return sz;
		}
	} ACS;
	typedef struct FLOAT_ARRAY {
		string id{};
		u64 count{};
		vector<float> arr{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			count = stoi(prsstr(_s, "count"));
			_s.erase(0u, 1u);
			_s.erase(_s.find("</float_array>"));
			const char* st = _s.c_str();
			vector<char> vch{};
			while (*st != '\0') {
				[[unlikely]]
				if (*st == ' ') {
					vch.push_back(0);
					vch.shrink_to_fit();
					arr.push_back((float)atof(vch.data()));
					vch.clear();
					st++;
					continue;
				}
				vch.push_back(*st++);
			}
			vch.shrink_to_fit();
			arr.push_back(stof(vch.data()));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += sizeof u64;
			sz += arr.size() * sizeof(float);
			return sz;
		}
	} FLTARR;
	typedef struct INT_ARRAY {
		string id{};
		u64 count{};
		vector<u64> arr{};

		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += sizeof u64;
			sz += arr.size() * sizeof u64;
			return sz;
		}
	} INTARR;
	typedef struct NAME_ARRAY {
		string id{};
		u64 count{};
		vector<string> arr{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			count = stoi(prsstr(_s, "count"));
			_s.erase(0u, _s.find(">") + 1);
			_s.erase(_s.find("</Name_array>"), _s.size());
			const char* st = _s.c_str();
			vector<char> vtmp{};
			while (*st != 0) {
				[[unlikely]]
				if (*st == ' ') {
					vtmp.push_back(0);
					arr.push_back(vtmp.data());
					vtmp.clear();
					vtmp.shrink_to_fit();
					st++;
					continue;
				}
				vtmp.push_back(*st++);
			}
			vtmp.push_back(0);
			arr.push_back(vtmp.data());
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += sizeof u64;
			for (auto& e : arr) sz += e.size();
			return sz;
		}
	} NMARR;
	typedef struct Source {
		string id{};

		FLTARR fltarr{};
		INTARR intarr{};
		NMARR namearr{};

		ACS accessor{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			if (srchstr(_s, "<float_array")) fltarr.prs(gensub2(_s, "<float_array"));
			else if (srchstr(_s, "<Name_array")) namearr.prs(gensub2(_s, "<Name_array"));
			accessor.prs(gensub2(_s, "<accessor"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += fltarr.size();
			sz += intarr.size();
			sz += namearr.size();
			sz += accessor.size();
			return sz;
		}
	} SRC;
	struct Input {
		string semantic{};
		string source{};
		u64 offset{};
		u64 set{};

		void prs(string _s) {
			semantic = prsstr(_s, "semantic");
			source = prsstr(_s, "source").erase(0u, 1u);
			if (srchstr(_s, "offset")) offset = stoi(prsstr(_s, "offset"));
			if (srchstr(_s, "set")) set = stoi(prsstr(_s, "set"));
		}
		u64 size() {
			u64 sz{};
			sz += semantic.size();
			sz += source.size();
			sz += sizeof u64;
			sz += sizeof u64;
			return sz;
		}
	};
	typedef struct Triangle {
		string material{};
		u64 count{};
		vector<Input> inputs{};
		vector<u32> indices{};

		void prs(string _s) {
			if (srchstr(_s, "material")) material = prsstr(_s, "material");
			count = stoi(prsstr(_s, "count"));
			inputs.resize(srchstr(_s, "<input"));
			for (auto& e : inputs) e.prs(gensub3(_s, "<input"));
			_s.erase(0u, _s.find("<p>") + 3);
			_s.erase(_s.find("</p>"), _s.size());
			const char* st = _s.c_str();
			u32 val{};
			while (*st != 0) {
				[[unlikely]]
				if (*st == ' ') {
					indices.push_back(val);
					val = 0;
					st++;
					continue;
				}
				val = val * 10 + (*st++ - '0');
			}
			indices.push_back(val);
		}
		u64 size() {
			u64 sz{};
			sz += material.size();
			sz += sizeof u64;
			for (auto& e : inputs) sz += e.size();
			sz += indices.size() * sizeof u32;
			return sz;
		}
	} Tri;
	typedef struct VertexInput {
		string id{};
		Input input{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			input.prs(gensub3(_s, "<input"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += input.size();
			return sz;
		}
	} VI;
	struct Mesh {
		vector<SRC> sources{};
		VI vi{};
		vector<Tri> tris{};

		void prs(string _s) {
			sources.resize(srchstr(_s, "<source"));
			for (auto& e : sources) e.prs(gensub2(_s, "<source"));
			vi.prs(gensub2(_s, "<vertices"));
			tris.resize(srchstr(_s, "<triangles"));
			for (auto& e : tris) e.prs(gensub2(_s, "<triangles"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : sources) sz += e.size();
			sz += vi.size();
			for (auto& e : tris) sz += e.size();
			return sz;
		}
	};
	struct Geometry {
		string id{};
		string name{};
		Mesh mesh{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");
			mesh.prs(gensub(_s, "<mesh>"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			sz += mesh.size();
			return sz;
		}
	};
	typedef struct LibraryGeometry {
		vector<Geometry> geos{};

		void prs(string _s) {
			geos.resize(srchstr(_s, "<geometry"));
			for (auto& e : geos) e.prs(gensub2(_s, "<geometry"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : geos) sz += e.size();
			return sz;
		}
	} LibGEO;
	typedef struct VertexWeight {
		u64 count{};
		vector<Input> inputs{};
		vector<u64> vcount{};
		vector<u64> v{};

		void prs(string _s) {
			count = stoi(prsstr(_s, "count"));
			inputs.resize(srchstr(_s, "<input"));
			for (auto& e : inputs) e.prs(gensub3(_s, "<input"));
			string vcs = _s;
			vcs.erase(0u, vcs.find("<vcount>") + 8);
			vcs.erase(vcs.find(" </vcount>"), vcs.size());
			const char* st = vcs.c_str();
			u32 val{};
			while (*st != 0) {
				[[unlikely]]
				if (*st == ' ') {
					vcount.push_back(val);
					val = 0;
					st++;
					continue;
				}
				val = val * 10 + (*st++ - '0');
			}
			vcount.push_back(val);
			val = 0;
			string vs = _s;
			vs.erase(0u, vs.find("<v>") + 3);
			vs.erase(vs.find("</v>"), vs.size());
			st = vs.c_str();
			while (*st != 0) {
				[[unlikely]]
				if (*st == ' ') {
					v.push_back(val);
					val = 0;
					st++;
					continue;
				}
				val = val * 10 + (*st++ - '0');
			}
			v.push_back(val);
		}
		u64 size() {
			u64 sz{};
			sz += sizeof u64;
			for (auto& e : inputs) sz += e.size();
			sz += vcount.size() * sizeof u64;
			sz += v.size() * sizeof u64;
			return sz;
		}
	} VW;
	typedef struct Joint {
		vector<Input> inputs{};

		void prs(string _s) {
			inputs.resize(srchstr(_s, "<input"));
			for (auto& e : inputs) e.prs(gensub3(_s, "<input"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : inputs) sz += e.size();
			return sz;
		}
	} JT;
	struct Skin {
		string source{};
		MTX bsm{};
		vector<SRC> sources{};
		JT joints{};
		VW weights{};

		void prs(string _s) {
			source = prsstr(_s, "source").erase(0u, 1u);


			string sub = _s;
			sub.erase(0u, sub.find("<bind_shape_matrix>") + 19);
			sub.erase(sub.find("</bind_shape_matrix>"), sub.size());
			const char* st = sub.c_str();
			vector<float> arr{};
			vector<char> vch{};
			while (*st != 0) {
				[[unlikely]]
				if (*st == ' ') {
					vch.push_back(0);
					vch.shrink_to_fit();
					arr.push_back(stof(vch.data()));
					vch.clear();
					st++;
					continue;
				}
				vch.push_back(*st++);
			}
			vch.push_back(0);
			vch.shrink_to_fit();
			arr.push_back(stod(vch.data()));
			bsm.e11 = arr[0]; bsm.e12 = arr[1]; bsm.e13 = arr[2]; bsm.e14 = arr[3];
			bsm.e21 = arr[4]; bsm.e22 = arr[5]; bsm.e23 = arr[6]; bsm.e24 = arr[7];
			bsm.e31 = arr[8]; bsm.e32 = arr[9]; bsm.e33 = arr[10]; bsm.e34 = arr[11];
			bsm.e41 = arr[12]; bsm.e42 = arr[13]; bsm.e43 = arr[14]; bsm.e44 = arr[15];

			sources.resize(srchstr(_s, "<source"));
			for (auto& e : sources) e.prs(gensub2(_s, "<source"));
			joints.prs(gensub(_s, "<joints>"));
			weights.prs(gensub2(_s, "<vertex_weights"));
		}
		u64 size() {
			u64 sz{};
			sz += source.size();
			sz += sizeof MTX;
			sz += sizeof u64;
			for (auto& e : sources) sz += e.size();
			sz += joints.size();
			sz += weights.size();
			return sz;
		}
	};
	typedef struct Controller {
		string id{};
		string name{};
		vector<Skin> skins{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");

			skins.resize(srchstr(_s, "<skin"));
			for (auto& e : skins) e.prs(gensub2(_s, "<skin"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			for (auto& e : skins) sz += e.size();
			return sz;
		}
	} CTRL;
	typedef struct LibraryController {
		vector<CTRL> ctrls{};

		void prs(string _s) {
			ctrls.resize(srchstr(_s, "<controller"));
			for (auto& e : ctrls) e.prs(gensub2(_s, "<controller"));
		}
		u64 size() {
			u64 sz{};
			for (auto& e : ctrls) sz += e.size();
			return sz;
		}
	} LibCTL;
	typedef struct Channel {
		string source{};
		string target{};

		void prs(string _s) {
			source = prsstr(_s, "source").erase(0u, 1u);
			target = prsstr(_s, "target");
		}
		u64 size() {
			u64 sz{};
			sz += source.size();
			sz += target.size();
			return sz;
		}
	} CHN;
	typedef struct Sampler {
		string id{};
		vector<Input> inputs{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			inputs.resize(srchstr(_s, "<input"));
			for (auto& e : inputs) e.prs(gensub3(_s, "<input"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			for (auto& e : inputs) sz += e.size();
			return sz;
		}
	} SMP;
	typedef struct JointAnimation {
		string id{};
		string name{};
		vector<SRC> sources{};
		SMP sampler{};
		CHN channel{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");
			sources.resize(srchstr(_s, "<source"));
			for (auto& e : sources) e.prs(gensub2(_s, "<source"));
			sampler.prs(gensub2(_s, "<sampler"));
			channel.prs(gensub3(_s, "<channel"));
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			for (auto& e : sources) sz += e.size();
			sz += sampler.size();
			sz += channel.size();
			return sz;
		}
	} JTAni;
	typedef struct Animation {
		string id{};
		string name{};
		vector<JTAni> jointanims{};

		void prs(string _s) {
			id = prsstr(_s, "id");
			name = prsstr(_s, "name");
			jointanims.resize(srchstr(_s, ">      <animation"));
			for (auto& e : jointanims) {
				string sub = _s;
				sub.erase(0u, sub.find(">      <animation") + 7);
				sub.erase(sub.find(">      </animation>") + 19, sub.size());
				e.prs(sub);
				_s.erase(0u, _s.find(">      </animation>") + 19);
			}
		}
		u64 size() {
			u64 sz{};
			sz += id.size();
			sz += name.size();
			for (auto& e : jointanims) sz += e.size();
			return sz;
		}
	} Ani;
	typedef struct LibraryAnimation {
		vector<Ani> animations{};

		void prs(string _s) {
			animations.resize(srchstr(_s, ">    <animation"));
			for (auto& e : animations) {
				string sub = _s;
				sub.erase(0u, sub.find(">    <animation") + 5);
				sub.erase(sub.find(">    </animation>") + 17, sub.size());
				e.prs(sub);
				_s.erase(0u, _s.find(">    </animation>") + 17);
			}
		}
		u64 size() {
			u64 sz{};
			for (auto& e : animations) sz += e.size();
			return sz;
		}
	} LibANI;
	typedef struct ColladaData {
		string xmlns{};
		string version{};
		string xmlns_xsi{};
		AST asset{};
		LibFX fx{};
		LibIMG img{};
		LibMTL mat{};
		LibGEO geo{};
		LibCTL ctrl{};
		LibANI anim{};

		void prs(const pugi::xml_node& node) {

			xmlns=node.attribute("xmlns").as_string();
			version=node.attribute("version").as_string();
			xmlns_xsi=node.attribute("xmlns:xsi").as_string();
			asset.prs(node.child("asset"));
			fx.prs(node.child("library_effects"));

			/*xmlns = prsstr(_s, "xmlns");
			version = prsstr(_s, "version");
			xmlns_xsi = prsstr(_s, "xmlns:xsi");
			asset.prs(gensub(_s, "<asset>"));
			if (srchstr(_s, "<library_effects>")) fx.prs(gensub(_s, "<library_effects>"));
			if (srchstr(_s, "<library_images>")) img.prs(gensub(_s, "<library_images>"));
			if (srchstr(_s, "<library_materials>")) mat.prs(gensub(_s, "<library_materials>"));
			if (srchstr(_s, "<library_geometries>")) geo.prs(gensub(_s, "<library_geometries>"));
			if (srchstr(_s, "<library_controllers>")) ctrl.prs(gensub(_s, "<library_controllers>"));
			if (srchstr(_s, "<library_animations>")) anim.prs(gensub(_s, "<library_animations>"));*/
		}
		u64 size() {
			u64 sz{};
			sz += xmlns.size();
			sz += version.size();
			sz += xmlns_xsi.size();
			sz += asset.size();
			sz += fx.size();
			sz += img.size();
			sz += mat.size();
			sz += geo.size();
			sz += ctrl.size();
			sz += anim.size();
			return sz;
		}
	} CD;
} using namespace Collada;

int main() {
	string fp{}, s{};
	getline(FS("cfg/path.txt"), fp);
	cout << "[PATH] " << fp << '\n';
	pugi::xml_document doc{};
	if(!doc.load_file(fp.c_str())) return 1;
	CD data{};
	data.prs(doc.child("COLLADA"));
	double sz = data.size();
	u8 exp{};
	for(;sz>=1024;exp++)sz /= 1024;
	switch (exp) {
		case 0:
			cout << "Size: " << sz << " Bytes";
			break;
		case 1:
			cout << "Size: " << sz << " KB";
			break;
		case 2:
			cout << "Size: " << sz << " MB";
			break;
		case 3:
			cout << "Size: " << sz << " GB";
			break;
	}
	return 0;
}