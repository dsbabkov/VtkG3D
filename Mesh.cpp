#include "Mesh.h"
#include <fstream>

Mesh readMesh(const char *path)
{
	std::ifstream is
			("/home/dmitriy/qtprojects/poligonqt/msqt/examples/cylinders_cast.g3d");
	unsigned nodeCount, elementCount;
	is >> nodeCount >> elementCount >> nodeCount;

	std::map<unsigned, Node> nodes;
	for (unsigned nodeNumber = 0; nodeNumber < nodeCount; ++nodeNumber) {
		unsigned incrementedNodeNumber;
		auto &[_, node] = *nodes.emplace_hint(nodes.cend(), nodeNumber, Node{});
		is >> node.x >> node.y >> node.z >> incrementedNodeNumber;
	}

	std::map<unsigned, Element> elements;
	for (unsigned elementNumber = 0; elementNumber < elementCount;
			++elementNumber) {
		auto &[_, element] = *elements.emplace_hint(elements.cend(), elementNumber, Element{});
		unsigned volumeIndex;
		is >> volumeIndex;

		for (auto &nodeNumber: element.nodeNumbers) {
			is >> nodeNumber;
			nodeNumber--;
		}

		unsigned incrementedElementNumber;
		is >> incrementedElementNumber;
	}

	std::map<unsigned, Volume> volumes;
	for (auto &[elementNumber, element]: elements) {
		int faceIndex;
		unsigned parsedElementNumber;
		unsigned volumeNumber;
		is >> volumeNumber >> faceIndex >> faceIndex >> faceIndex >> faceIndex
				>> parsedElementNumber;
		auto &volume = volumes[volumeNumber];
		volume.elementNumbers.emplace_hint(volume.elementNumbers.cend(), elementNumber);
	}

	Mesh result;
	for (auto &[volumeNumber, volume]: volumes) {
		volume.mesh = &result;
	}

	result.volumes = std::move(volumes);
	result.elements = std::move(elements);
	result.nodes = std::move(nodes);

	return result;
}

Mesh::Mesh(Mesh &&other) noexcept: nodes(std::move(other.nodes))
		, elements(std::move(other.elements))
		, volumes(std::move(other.volumes))
{
	for (auto &[volumeNumber, volume]: volumes) {
		volume.mesh = this;
	}
}
