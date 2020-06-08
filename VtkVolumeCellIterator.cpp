#include "VtkVolumeCellIterator.h"
#include "VtkVolume.h"
#include "Mesh.h"

bool VtkVolumeCellIterator::IsDoneWithTraversal()
{
	return elementNumbersIt_ == impl_->volume->elementNumbers.cend();
}

vtkIdType VtkVolumeCellIterator::GetCellId()
{
	return *elementNumbersIt_;
}

void VtkVolumeCellIterator::ResetToFirstCell()
{
	elementNumbersIt_ = impl_->volume->elementNumbers.cbegin();
}

void VtkVolumeCellIterator::IncrementToNextCell()
{
	++elementNumbersIt_;
}

void VtkVolumeCellIterator::FetchCellType()
{
	CellType = VTK_TETRA;
}

void VtkVolumeCellIterator::FetchPointIds()
{
	PointIds->Reset();
	const auto *volume = impl_->volume;
	const auto *mesh = volume->mesh;
	const auto &element = mesh->elements.at(*elementNumbersIt_);
	for (const auto nodeNumber: element.nodeNumbers) {
		PointIds->InsertNextId(nodeNumber);
	}
}

void VtkVolumeCellIterator::FetchPoints()
{
	const auto *volume = impl_->volume;
	const auto *mesh = volume->mesh;
	const auto &nodes = mesh->nodes;

	Points->Reset();
	for (int i = 0; i < 4; ++i) {
		const auto &node = nodes.at(PointIds->GetId(i));
		Points->InsertNextPoint(node.x, node.y, node.z);
	}
}

void VtkVolumeCellIterator::SetMappedUnstructuredGrid(vtkMappedUnstructuredGrid<VolumeVtkImplementation, VtkVolumeCellIterator> *grid)
{
	impl_ = grid->GetImplementation();
}

VtkVolumeCellIterator *VtkVolumeCellIterator::New()
{
	VtkVolumeCellIterator *pIter = new VtkVolumeCellIterator;
	//pIter->PointIds = vtkIdList
	return pIter;
}
