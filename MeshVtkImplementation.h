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
	vtkIdType GetNumberOfCells();

	int GetCellType(vtkIdType cellId);

	void GetCellPoints(vtkIdType cellId, vtkIdList *ptIds);

	void GetPointCells(vtkIdType ptId, vtkIdList *cellIds);

	int GetMaxCellSize();

	void GetIdsOfCellsOfType(int type, vtkIdTypeArray *array);

	int IsHomogeneous();

	void Allocate(vtkIdType numCells, int extSize = 1000);

	vtkIdType InsertNextCell(int type, vtkIdList *ptIds);

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[]);

	vtkIdType InsertNextCell(int type, vtkIdType npts, const vtkIdType ptIds[], vtkIdType nfaces, const vtkIdType faces[]);

	void ReplaceCell(vtkIdType cellId, int npts, const vtkIdType pts[]);

public:
	Volume *volume;
};

