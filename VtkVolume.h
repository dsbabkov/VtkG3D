#pragma once

#include "VolumeVtkImplementation.h"
#include "VtkVolumeCellIterator.h"
#include <vtkMappedUnstructuredGrid.h>


/* В vtk8 какие-то странности с vtkMakeMappedUnstructuredGridWithIter
 * Принимает 4 аргумента, включая _exportDecl и разворачивается
 * в некорректный код
 */

class VtkVolume:
		public vtkMappedUnstructuredGrid<VolumeVtkImplementation,
				VtkVolumeCellIterator>
{
public:
	using VtkVolumeSuperclass = vtkMappedUnstructuredGrid<VolumeVtkImplementation, VtkVolumeCellIterator>;
	vtkTypeMacro(VtkVolume, VtkVolumeSuperclass);
	static VtkVolume *New();
protected:
	VtkVolume()
	{
		VolumeVtkImplementation *i = VolumeVtkImplementation::New();
		this->SetImplementation(i);
		i->Delete();
	}
	~VtkVolume() override
	{}
private:
	VtkVolume(const VtkVolume &);
	void operator=(const VtkVolume &);
};