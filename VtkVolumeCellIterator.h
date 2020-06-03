#pragma once

#include <vtkCellIterator.h>
#include <vtkSmartPointer.h>
#include <set>

class VtkVolume;
class VolumeVtkImplementation;

template <class Implementation, class CellIterator>
class vtkMappedUnstructuredGrid;

class VtkVolumeCellIterator: public vtkCellIterator
{
public:
	vtkTypeMacro(VtkVolumeCellIterator, vtkCellIterator)
	static VtkVolumeCellIterator *New();

	void SetMappedUnstructuredGrid(vtkMappedUnstructuredGrid<VolumeVtkImplementation, VtkVolumeCellIterator> *grid);
	bool IsDoneWithTraversal() override;
	vtkIdType GetCellId() override;

protected:
	void ResetToFirstCell() override;
	void IncrementToNextCell() override;
	void FetchCellType() override;
	void FetchPointIds() override;
	void FetchPoints() override;

private:
	vtkSmartPointer<VolumeVtkImplementation> impl_;
	std::set<unsigned>::const_iterator elementNumbersIt_;
};
