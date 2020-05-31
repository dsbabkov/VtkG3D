#pragma once

#include <vtkObject.h>
#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include "Mesh.h"

class Volume;

class MeshVtkImplementation: public vtkObject
{
	vtkTypeMacro(MeshVtkImplementation, vtkObject);
public:
	static MeshVtkImplementation *New() {
		std::cout << "new MeshVtkImplementation created\n";
		VTK_STANDARD_NEW_BODY(MeshVtkImplementation)
	}
	vtkIdType GetNumberOfCells() {
		std::cout << "GetNumberOfCells: " << volume->elementNumbers.size() << '\n';
		return volume->elementNumbers.size();
	}

	int GetCellType(vtkIdType cellId) {
		std::cout << "GetCellType: VTK_TETRA\n";
		return VTK_TETRA;
	}

	void GetCellPoints(vtkIdType cellId, vtkIdList *ptIds) {
		std::cout << "GetCellPoints\n";
		auto it = volume->elementNumbers.begin();
		while(cellId--) {
			it++;
		}
		const auto elementNumber = *it;
		const auto element = volume->mesh->elements.at(elementNumber);

		ptIds->Reset();
		ptIds->InsertNextId(element.nodeNumbers[0]);
		ptIds->InsertNextId(element.nodeNumbers[1]);
		ptIds->InsertNextId(element.nodeNumbers[2]);
		ptIds->InsertNextId(element.nodeNumbers[3]);
	}

	void GetPointCells(vtkIdType ptId, vtkIdList *cellIds) {
		std::cout << "GetPointCells\n";
		unsigned nodeNumber = ptId;
		std::set<unsigned> elementNumbersContainingNode;
		for (const auto &[elementNumber, element]: volume->mesh->elements) {
			for (const auto elementNodeNumber: element.nodeNumbers) {
				if (elementNodeNumber == elementNumber) {
					elementNumbersContainingNode.insert(elementNumber);
				}
			}
		}

		for (auto it = elementNumbersContainingNode.begin(); it != elementNumbersContainingNode.end();) {
			const auto elementNumber = *it;
			if (volume->elementNumbers.find(elementNumber) == volume->elementNumbers.end()) {
				it = elementNumbersContainingNode.erase(it);
			}
			else {
				++it;
			}
		}

		cellIds->Reset();
		vtkIdType vtkCellId = 0;
		for (const auto elementNumber: volume->elementNumbers) {
			if (elementNumbersContainingNode.find(elementNumber) != elementNumbersContainingNode.cend()) {
				cellIds->InsertNextId(vtkCellId);
			}
			++vtkCellId;
		}

	}

	int GetMaxCellSize() {
		std::cout << "GetMaxCellSize\n";
		return 4;
	}

	void GetIdsOfCellsOfType(int type, vtkIdTypeArray *array) {
		assert("Not implemented" == nullptr);
	}

	int IsHomogeneous() {
		std::cout << "IsHomogeneous\n";
		return true;
	}

	void Allocate(vtkIdType numCells, int extSize = 1000) {
		assert("Not implemented" == nullptr);
	}

	vtkIdType InsertNextCell(int type, vtkIdList *ptIds) {
		assert("Not implemented" == nullptr);
	}

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[]) {
		assert("Not implemented" == nullptr);
	}

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[]) {
		assert("Not implemented" == nullptr);
	}

	void ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[]) {
		assert("Not implemented" == nullptr);
	}

public:
	Volume *volume;
};

