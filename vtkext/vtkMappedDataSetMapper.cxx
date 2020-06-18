#include "vtkMappedDataSetMapper.h"

// За основу взят файл:
// ${VTK_SOURCE_ROOT}/Rendering/Core/vtkDataSetMapper.cxx

#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkScalarsToColors.h"

#include "vtkMappedDataSetSurfaceFilter.h"

vtkStandardNewMacro(vtkMappedDataSetMapper);

//----------------------------------------------------------------------------
vtkMappedDataSetMapper::vtkMappedDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
}

//----------------------------------------------------------------------------
vtkMappedDataSetMapper::~vtkMappedDataSetMapper()
{
  // delete internally created objects.
  if ( this->GeometryExtractor )
  {
    this->GeometryExtractor->Delete();
  }
  if ( this->PolyDataMapper )
  {
    this->PolyDataMapper->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkMappedDataSetMapper::SetInputData(vtkDataSet *input)
{
  this->SetInputDataInternal(0, input);
}

//----------------------------------------------------------------------------
vtkDataSet *vtkMappedDataSetMapper::GetInput()
{
  return this->Superclass::GetInputAsDataSet();
}

//----------------------------------------------------------------------------
void vtkMappedDataSetMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->PolyDataMapper)
  {
    this->PolyDataMapper->ReleaseGraphicsResources( renWin );
  }
}

//----------------------------------------------------------------------------
// Receives from Actor -> maps data to primitives
//
void vtkMappedDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  // make sure that we've been properly initialized
  //
  if ( !this->GetInput() )
  {
    vtkErrorMacro(<< "No input!\n");
    return;
  }

  // Need a lookup table
  //
  if ( this->LookupTable == NULL )
  {
    this->CreateDefaultLookupTable();
  }
  this->LookupTable->Build();

  // Now can create appropriate mapper
  //
  if ( this->PolyDataMapper == NULL )
  {
    vtkMappedDataSetSurfaceFilter *gf = vtkMappedDataSetSurfaceFilter::New();
    vtkPolyDataMapper *pm = vtkPolyDataMapper::New();
    pm->SetInputConnection(gf->GetOutputPort());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
  }

  // share clipping planes with the PolyDataMapper
  //
  if (this->ClippingPlanes != this->PolyDataMapper->GetClippingPlanes())
  {
    this->PolyDataMapper->SetClippingPlanes(this->ClippingPlanes);
  }

  // For efficiency: if input type is vtkPolyData, there's no need to
  // pass it through the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_POLY_DATA )
  {
    this->PolyDataMapper->SetInputConnection(
      this->GetInputConnection(0, 0));
  }
  else
  {
    this->GeometryExtractor->SetInputData(this->GetInput());
    this->PolyDataMapper->SetInputConnection(
      this->GeometryExtractor->GetOutputPort());
  }

  // update ourselves in case something has changed
  this->PolyDataMapper->SetLookupTable(this->GetLookupTable());
  this->PolyDataMapper->SetScalarVisibility(this->GetScalarVisibility());
  this->PolyDataMapper->SetUseLookupTableScalarRange(
    this->GetUseLookupTableScalarRange());
  this->PolyDataMapper->SetScalarRange(this->GetScalarRange());
//  this->PolyDataMapper->SetImmediateModeRendering(
//    this->GetImmediateModeRendering());
  this->PolyDataMapper->SetColorMode(this->GetColorMode());
  this->PolyDataMapper->SetInterpolateScalarsBeforeMapping(
                               this->GetInterpolateScalarsBeforeMapping());

  this->PolyDataMapper->SetScalarMode(this->GetScalarMode());
  if ( this->ScalarMode == VTK_SCALAR_MODE_USE_POINT_FIELD_DATA ||
       this->ScalarMode == VTK_SCALAR_MODE_USE_CELL_FIELD_DATA )
  {
    if ( this->ArrayAccessMode == VTK_GET_ARRAY_BY_ID )
    {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayId,ArrayComponent);
    }
    else
    {
      this->PolyDataMapper->ColorByArrayComponent(this->ArrayName,ArrayComponent);
    }
  }

  this->PolyDataMapper->Render(ren,act);
  this->TimeToDraw = this->PolyDataMapper->GetTimeToDraw();
}

//----------------------------------------------------------------------------
void vtkMappedDataSetMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->PolyDataMapper )
  {
    os << indent << "Poly Mapper: (" << this->PolyDataMapper << ")\n";
  }
  else
  {
    os << indent << "Poly Mapper: (none)\n";
  }

  if ( this->GeometryExtractor )
  {
    os << indent << "Geometry Extractor: (" << this->GeometryExtractor << ")\n";
  }
  else
  {
    os << indent << "Geometry Extractor: (none)\n";
  }
}

//----------------------------------------------------------------------------
vtkMTimeType vtkMappedDataSetMapper::GetMTime()
{
  vtkMTimeType mTime=this->vtkMapper::GetMTime();
  vtkMTimeType time;

  if ( this->LookupTable != NULL )
  {
    time = this->LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
  }

  return mTime;
}

//----------------------------------------------------------------------------
int vtkMappedDataSetMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void vtkMappedDataSetMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  // These filters share our input and are therefore involved in a
  // reference loop.
  vtkGarbageCollectorReport(collector, this->GeometryExtractor,
                            "GeometryExtractor");
  vtkGarbageCollectorReport(collector, this->PolyDataMapper,
                            "PolyDataMapper");
}
