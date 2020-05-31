#include "MeshVtkImplementation.h"
#include "Mesh.h"
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>

vtkIdType MeshVtkImplementation::GetNumberOfCells() const
{
	return volume->elementNumbers.size();
}

void MeshVtkImplementation::GetCellPoints(vtkIdType cellId, vtkIdList *ptIds) const
{
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

void MeshVtkImplementation::GetPointCells(vtkIdType ptId, vtkIdList *cellIds) const
{
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

void MeshVtkImplementation::GetIdsOfCellsOfType(int type, vtkIdTypeArray *array) const
{
	assert("Not implemented" == nullptr);
}

void MeshVtkImplementation::Allocate(vtkIdType numCells, int extSize)
{
	assert("Not implemented" == nullptr);
}

vtkIdType MeshVtkImplementation::InsertNextCell(int type, vtkIdList *ptIds)
{
	assert("Not implemented" == nullptr);
}

void
MeshVtkImplementation::ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[])
{
	assert("Not implemented" == nullptr);
}

vtkIdType MeshVtkImplementation::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[])
{
	assert("Not implemented" == nullptr);
}

vtkIdType MeshVtkImplementation::InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[])
{
	assert("Not implemented" == nullptr);
}

MeshVtkImplementation *MeshVtkImplementation::New()
{
	VTK_STANDARD_NEW_BODY(MeshVtkImplementation)
}
