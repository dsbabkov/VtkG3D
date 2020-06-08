#include "Mesh.h"
#include <fstream>
#include <iostream>

Mesh readMesh(const char *path)
{
	std::ifstream is(path);
			//("/home/dmitriy/qtprojects/poligonqt/msqt/examples/cylinders_cast.g3d");
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
		unsigned int volumeNumber;
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

void Mesh::removeVolume(unsigned int volumeNumber)
{
	const Volume &volume = volumes.at(volumeNumber);
	for (const auto elementNumber: volume.elementNumbers) {
		elements.erase(elementNumber);
	}
	volumes.erase(volumeNumber);
#if 1
	std::set<unsigned> orphanedNodes;
	for (const auto &[nodeNumber, node]: nodes) {
		orphanedNodes.insert(nodeNumber);
	}
	for (const auto &[elementNumber, element]: elements) {
		for (const auto nodeNumber: element.nodeNumbers) {
			orphanedNodes.erase(nodeNumber);
		}
	}
	for (const auto nodeNumber: orphanedNodes) {
		nodes.erase(nodeNumber);
	}
	std::cout << "orphaned nodes removed: " << orphanedNodes.size() << std::endl;
#endif
}
