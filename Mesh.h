#pragma once

#include <map>
#include <set>

struct Node {
	float x, y, z;
};

struct Element {
	std::array<unsigned, 4> nodeNumbers;
};

struct Mesh;
struct Volume {
	Mesh *mesh;
	std::set<unsigned> elementNumbers;
};

struct Mesh {
	std::map<unsigned, Node> nodes;
	std::map<unsigned, Element> elements;
	std::map<unsigned, Volume> volumes;

	Mesh() = default;
	Mesh (const Mesh &) = delete;
	Mesh (Mesh &&other) noexcept;

	void removeVolume(unsigned volumeNumber);
};

Mesh readMesh(const char *path);