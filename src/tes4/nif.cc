#include "nif.hh"

#include "../engine/mesh.hh"

#include <sstream>

// >:) 
#define private public
#define protected public

#include <niflib/obj/NiTriShape.h>
#include <niflib/obj/NiTriStrips.h>
#include <niflib/obj/NiTriShapeData.h>
#include <niflib/obj/NiTriStripsData.h>
#include <niflib/obj/NiObject.h>
#include <niflib/niflib.h>
#include <niflib/obj/BSFadeNode.h>

// https://stackoverflow.com/a/67457982
class vectorbuf : public std::streambuf {
public:
	vectorbuf(std::vector<uint8_t> const &v) {
		setg((char*)v.data(), (char*)v.data(), (char*)(v.data() + v.size()));
	}
};

void load_nif_add_mesh(Niflib::Ref<Niflib::NiObject> node, engine::object &obj) {
	Niflib::Ref<Niflib::NiTriBasedGeom> geom = Niflib::DynamicCast<Niflib::NiTriBasedGeom>(node);
	if (!geom)
		throw std::runtime_error("tes4::load_nif impossible error.");
	
	auto ni_transform = geom->GetLocalTransform();
	glm::mat4 m(*(glm::mat4*)&ni_transform);

	auto data = geom->GetData();

	engine::mesh &mesh = obj.add_mesh();
	mesh.vertices.resize(data->vertices.size());

	// TODO: use multiple uv sets
	bool has_cols = data->vertexColors.size();
	bool has_nrms = data->normals.size();
	bool has_uvs = data->uvSets.size() && data->uvSets[0].size();

	for (int i = 0; i < data->vertices.size(); ++i) {
		mesh.vertices[i] = {
			*(glm::vec3 *)&data->vertices[i],
			has_cols ? *(glm::vec4 *)&data->vertexColors[i] : glm::vec4(1.f),
			has_nrms ? *(glm::vec3 *)&data->normals[i] : glm::vec3(0.f, 1.f, 0.f),
			has_uvs ? *(glm::vec2 *)&data->uvSets[0][i] : glm::vec2(0.f)
		};
	}
	
	std::string n = node->GetType().GetTypeName();

	if (n == "NiTriShape") {
		mesh.draw_mode = GL_TRIANGLES;
		
		auto data1 = Niflib::DynamicCast<Niflib::NiTriShapeData>(data);

		mesh.indices.reserve(data1->triangles.size() * 3);
		for (auto &t : data1->triangles) { mesh.indices.push_back(t.v1); mesh.indices.push_back(t.v2); mesh.indices.push_back(t.v3); }
	}
	
	mesh.build();
}

void load_nif_r(Niflib::Ref<Niflib::NiNode> node, engine::object &obj) {
	for (auto child : node->GetChildren()) {
		std::string n = child->GetType().GetTypeName();
		//std::printf("node %s\n", n.c_str());
		if (n == "NiNode")
			load_nif_r(Niflib::DynamicCast<Niflib::NiNode>(child), obj);
		else if (n == "NiTriStrips")
			load_nif_add_mesh(Niflib::DynamicCast<Niflib::NiObject>(child), obj);
		else if (n == "NiTriShape")
			load_nif_add_mesh(Niflib::DynamicCast<Niflib::NiObject>(child), obj);
	}
}

void tes4::load_nif(std::vector<uint8_t> const &data, std::string const &name, engine::object &obj) {
	vectorbuf vbuf(data);
	std::istream is(&vbuf);

	auto object = Niflib::ReadNifTree(is);
	auto root_node = Niflib::DynamicCast<Niflib::NiNode>(object);

	if (!root_node)
		throw std::runtime_error("tes4::load_nif can't load nif: invalid root node " + name);

	load_nif_r(root_node, obj);
}
