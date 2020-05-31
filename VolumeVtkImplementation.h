#pragma once

#include <vtkObject.h>
#include <vtkCellType.h>

class Volume;
class vtkIdList;
class vtkIdTypeArray;

class VolumeVtkImplementation: public vtkObject
{
	vtkTypeMacro(VolumeVtkImplementation, vtkObject);
public:
	static VolumeVtkImplementation *New();
	vtkIdType GetNumberOfCells() const;

	constexpr int GetCellType(vtkIdType /*cellId*/) const {
		return VTK_TETRA;
	}

	void GetCellPoints(vtkIdType cellId, vtkIdList *ptIds) const;

	void GetPointCells(vtkIdType ptId, vtkIdList *cellIds) const;

	constexpr int GetMaxCellSize() const {
		return 4;
	}

	void GetIdsOfCellsOfType(int type, vtkIdTypeArray *array) const;

	constexpr int IsHomogeneous() const {
		return true;
	}

	void Allocate(vtkIdType numCells, int extSize = 1000);

	vtkIdType InsertNextCell(int type, vtkIdList *ptIds);

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[]);

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[]);

	void ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[]);

public:
	Volume *volume;
};

