#include "vtkMappedDataSetSurfaceFilter.h"

// За основу взят файл:
// ${VTK_SOURCE_ROOT}/Filters/Geometry/vtkDataSetSurfaceFilter.cxx

// TODO: delete unnecessary headers
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellIterator.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkHexahedron.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPyramid.h"
#include "vtkRectilinearGrid.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGridGeometryFilter.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkTetra.h"
#include "vtkUniformGrid.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGridBase.h"
#include "vtkUnstructuredGridGeometryFilter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkVoxel.h"
#include "vtkWedge.h"
#include "vtkStructuredData.h"

#include <algorithm>

#include <cassert>

vtkStandardNewMacro(vtkMappedDataSetSurfaceFilter);

//----------------------------------------------------------------------------
vtkMappedDataSetSurfaceFilter::vtkMappedDataSetSurfaceFilter()
{
}

//----------------------------------------------------------------------------
vtkMappedDataSetSurfaceFilter::~vtkMappedDataSetSurfaceFilter()
{
}

//----------------------------------------------------------------------------
void vtkMappedDataSetSurfaceFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkMappedDataSetSurfaceFilter::UnstructuredGridExecute(vtkDataSet *dataSetInput,
                                                     vtkPolyData *output)
{
  vtkUnstructuredGridBase *input =
      vtkUnstructuredGridBase::SafeDownCast(dataSetInput);

  vtkSmartPointer<vtkCellIterator> cellIter =
      vtkSmartPointer<vtkCellIterator>::Take(input->NewCellIterator());

  // Before we start doing anything interesting, check if we need handle
  // non-linear cells using sub-division.
  bool handleSubdivision = false;
  if (this->NonlinearSubdivisionLevel >= 1)
  {
    // Check to see if the data actually has nonlinear cells.  Handling
    // nonlinear cells adds unnecessary work if we only have linear cells.
    vtkIdType numCells = input->GetNumberOfCells();
    if (input->IsHomogeneous())
    {
      if (numCells >= 1)
      {
        handleSubdivision = !vtkCellTypes::IsLinear(input->GetCellType(0));
      }
    }
    else
    {
      for (cellIter->InitTraversal(); !cellIter->IsDoneWithTraversal();
           cellIter->GoToNextCell())
      {
        if (!vtkCellTypes::IsLinear(cellIter->GetCellType()))
        {
          handleSubdivision = true;
          break;
        }
      }
    }
  }

  vtkSmartPointer<vtkUnstructuredGrid> tempInput;
  if (handleSubdivision)
  {
    // Since this filter only properly subdivides 2D cells past
    // level 1, we convert 3D cells to 2D by using
    // vtkUnstructuredGridGeometryFilter.
    vtkNew<vtkUnstructuredGridGeometryFilter> uggf;
    vtkNew<vtkUnstructuredGrid> clone;
    clone->ShallowCopy(input);
    uggf->SetInputData(clone.GetPointer());
    uggf->SetPassThroughCellIds(this->PassThroughCellIds);
    uggf->SetOriginalCellIdsName(this->GetOriginalCellIdsName());
    uggf->SetPassThroughPointIds(this->PassThroughPointIds);
    uggf->SetOriginalPointIdsName(this->GetOriginalPointIdsName());
    uggf->DuplicateGhostCellClippingOff();
    // Disable point merging as it may prevent the correct visualization
    // of non-continuous attributes.
    uggf->MergingOff();
    uggf->Update();

    tempInput = vtkSmartPointer<vtkUnstructuredGrid>::New();
    tempInput->ShallowCopy(uggf->GetOutputDataObject(0));
    input = tempInput;
    cellIter = vtkSmartPointer<vtkCellIterator>::Take(input->NewCellIterator());
  }

  vtkUnsignedCharArray* ghosts = input->GetPointGhostArray();
  vtkCellArray *newVerts;
  vtkCellArray *newLines;
  vtkCellArray *newPolys;
  vtkPoints *newPts;
  vtkIdType *ids;
  int progressCount;
  int i, j;
  int cellType;
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkIdType numCells=input->GetNumberOfCells();
  vtkGenericCell *cell;
  vtkIdList *pointIdList;
  vtkIdType *pointIdArray;
  vtkIdType *pointIdArrayEnd;
  int numFacePts, numCellPts;
  vtkIdType inPtId, outPtId;
  vtkPointData *inputPD = input->GetPointData();
  vtkCellData *inputCD = input->GetCellData();
  vtkFieldData *inputFD = input->GetFieldData();
  vtkCellData *cd = input->GetCellData();
  vtkPointData *outputPD = output->GetPointData();
  vtkCellData *outputCD = output->GetCellData();
  vtkFieldData *outputFD = output->GetFieldData();
  vtkFastGeomQuad *q;

  // Shallow copy field data not associated with points or cells
  outputFD->ShallowCopy(inputFD);

  // These are for the default case/
  vtkIdList *pts;
  vtkPoints *coords;
  vtkCell *face;
  int flag2D = 0;

  // These are for subdividing quadratic cells
  vtkDoubleArray *parametricCoords;
  vtkDoubleArray *parametricCoords2;
  vtkIdList *outPts;
  vtkIdList *outPts2;

  pts = vtkIdList::New();
  coords = vtkPoints::New();
  parametricCoords = vtkDoubleArray::New();
  parametricCoords2 = vtkDoubleArray::New();
  outPts = vtkIdList::New();
  outPts2 = vtkIdList::New();
  // might not be necessary to set the data type for coords
  // but certainly safer to do so
  coords->SetDataType(input->GetPoints()->GetData()->GetDataType());
  cell = vtkGenericCell::New();

  this->NumberOfNewCells = 0;
  this->InitializeQuadHash(this->EstimateNecessaryQuadHashSize(input));

  // Allocate
  //
  newPts = vtkPoints::New();
  newPts->SetDataType(input->GetPoints()->GetData()->GetDataType());
  newPts->Allocate(numPts);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(4*numCells,numCells/2);
  newVerts = vtkCellArray::New();
  newLines = vtkCellArray::New();

  if (handleSubdivision == false)
  {
    outputPD->CopyGlobalIdsOn();
    outputPD->CopyAllocate(inputPD, numPts, numPts/2);
  }
  else
  {
    outputPD->InterpolateAllocate(inputPD, numPts, numPts/2);
  }
  outputCD->CopyGlobalIdsOn();
  outputCD->CopyAllocate(inputCD, numCells, numCells/2);

  if (this->PassThroughCellIds)
  {
    this->OriginalCellIds = vtkIdTypeArray::New();
    this->OriginalCellIds->SetName(this->GetOriginalCellIdsName());
    this->OriginalCellIds->SetNumberOfComponents(1);
  }
  if (this->PassThroughPointIds)
  {
    this->OriginalPointIds = vtkIdTypeArray::New();
    this->OriginalPointIds->SetName(this->GetOriginalPointIdsName());
    this->OriginalPointIds->SetNumberOfComponents(1);
  }

  // First insert all points.  Points have to come first in poly data.
  for (cellIter->InitTraversal(); !cellIter->IsDoneWithTraversal();
       cellIter->GoToNextCell())
  {
    cellType = cellIter->GetCellType();

    // A couple of common cases to see if things go faster.
    if (cellType == VTK_VERTEX || cellType == VTK_POLY_VERTEX)
    {
      pointIdList = cellIter->GetPointIds();
      numCellPts = pointIdList->GetNumberOfIds();
      pointIdArray = pointIdList->GetPointer(0);
      pointIdArrayEnd = pointIdArray + numCellPts;
      newVerts->InsertNextCell(numCellPts);
      while (pointIdArray != pointIdArrayEnd)
      {
        outPtId = this->GetOutputPointId(*(pointIdArray++), input, newPts,
                                         outputPD);
        newVerts->InsertCellPoint(outPtId);
      }
      vtkIdType cellId = cellIter->GetCellId();
      this->RecordOrigCellId(this->NumberOfNewCells, cellId);
      outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
    }
  }

  // Traverse cells to extract geometry
  //
  progressCount = 0;
  int abort=0;
  vtkIdType progressInterval = numCells/20 + 1;

  // First insert all points lines in output and 3D geometry in hash.
  // Save 2D geometry for second pass.
  for(cellIter->InitTraversal(); !cellIter->IsDoneWithTraversal() && !abort;
      cellIter->GoToNextCell())
  {
    vtkIdType cellId = cellIter->GetCellId();
    //Progress and abort method support
    if ( progressCount >= progressInterval )
    {
      vtkDebugMacro(<<"Process cell #" << cellId);
      this->UpdateProgress (static_cast<double>(cellId)/numCells);
      abort = this->GetAbortExecute();
      progressCount = 0;
    }
    progressCount++;

    cellType = cellIter->GetCellType();
    switch (cellType)
    {
      case VTK_VERTEX:
      case VTK_POLY_VERTEX:
        // Do nothing -- these were handled previously.
        break;

      case VTK_LINE:
      case VTK_POLY_LINE:
        pointIdList = cellIter->GetPointIds();
        numCellPts = pointIdList->GetNumberOfIds();
        pointIdArray = pointIdList->GetPointer(0);
        pointIdArrayEnd = pointIdArray + numCellPts;

        newLines->InsertNextCell(numCellPts);
        while (pointIdArray != pointIdArrayEnd)
        {
          outPtId = this->GetOutputPointId(*(pointIdArray++), input, newPts,
                                           outputPD);
          newLines->InsertCellPoint(outPtId);
        }

        this->RecordOrigCellId(this->NumberOfNewCells, cellId);
        outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
        break;

      case VTK_HEXAHEDRON:
        pointIdList = cellIter->GetPointIds();
        ids = pointIdList->GetPointer(0);
        this->InsertQuadInHash(ids[0], ids[1], ids[5], ids[4], cellId);
        this->InsertQuadInHash(ids[0], ids[3], ids[2], ids[1], cellId);
        this->InsertQuadInHash(ids[0], ids[4], ids[7], ids[3], cellId);
        this->InsertQuadInHash(ids[1], ids[2], ids[6], ids[5], cellId);
        this->InsertQuadInHash(ids[2], ids[3], ids[7], ids[6], cellId);
        this->InsertQuadInHash(ids[4], ids[5], ids[6], ids[7], cellId);
        break;

      case VTK_VOXEL:
        pointIdList = cellIter->GetPointIds();
        ids = pointIdList->GetPointer(0);
        this->InsertQuadInHash(ids[0], ids[1], ids[5], ids[4], cellId);
        this->InsertQuadInHash(ids[0], ids[2], ids[3], ids[1], cellId);
        this->InsertQuadInHash(ids[0], ids[4], ids[6], ids[2], cellId);
        this->InsertQuadInHash(ids[1], ids[3], ids[7], ids[5], cellId);
        this->InsertQuadInHash(ids[2], ids[6], ids[7], ids[3], cellId);
        this->InsertQuadInHash(ids[4], ids[5], ids[7], ids[6], cellId);
        break;

      case VTK_TETRA:
        pointIdList = cellIter->GetPointIds();
        ids = pointIdList->GetPointer(0);
        this->InsertTriInHash(ids[0], ids[1], ids[3], cellId, 2);
        this->InsertTriInHash(ids[0], ids[2], ids[1], cellId, 3);
        this->InsertTriInHash(ids[0], ids[3], ids[2], cellId, 1);
        this->InsertTriInHash(ids[1], ids[2], ids[3], cellId, 0);
        break;

      case VTK_PENTAGONAL_PRISM:
        pointIdList = cellIter->GetPointIds();
        ids = pointIdList->GetPointer(0);
        this->InsertQuadInHash (ids[0], ids[1], ids[6], ids[5], cellId);
        this->InsertQuadInHash (ids[1], ids[2], ids[7], ids[6], cellId);
        this->InsertQuadInHash (ids[2], ids[3], ids[8], ids[7], cellId);
        this->InsertQuadInHash (ids[3], ids[4], ids[9], ids[8], cellId);
        this->InsertQuadInHash (ids[4], ids[0], ids[5], ids[9], cellId);
        this->InsertPolygonInHash(ids, 5, cellId);
        this->InsertPolygonInHash(&ids[5], 5, cellId);
        break;

      case VTK_HEXAGONAL_PRISM:
        pointIdList = cellIter->GetPointIds();
        ids = pointIdList->GetPointer(0);
        this->InsertQuadInHash(ids[0], ids[1], ids[7], ids[6], cellId);
        this->InsertQuadInHash(ids[1], ids[2], ids[8], ids[7], cellId);
        this->InsertQuadInHash(ids[2], ids[3], ids[9], ids[8], cellId);
        this->InsertQuadInHash(ids[3], ids[4], ids[10], ids[9], cellId);
        this->InsertQuadInHash(ids[4], ids[5], ids[11], ids[10], cellId);
        this->InsertQuadInHash(ids[5], ids[0], ids[6], ids[11], cellId);
        this->InsertPolygonInHash (ids, 6, cellId);
        this->InsertPolygonInHash (&ids[6], 6, cellId);
        break;

      case VTK_PIXEL:
      case VTK_QUAD:
      case VTK_TRIANGLE:
      case VTK_POLYGON:
      case VTK_TRIANGLE_STRIP:
      case VTK_QUADRATIC_TRIANGLE:
      case VTK_BIQUADRATIC_TRIANGLE:
      case VTK_QUADRATIC_QUAD:
      case VTK_QUADRATIC_LINEAR_QUAD:
      case VTK_BIQUADRATIC_QUAD:
      case VTK_QUADRATIC_POLYGON:
        // save 2D cells for third pass
        flag2D = 1;
        break;

      default:
      {
        // Default way of getting faces. Differentiates between linear
        // and higher order cells.
        cellIter->GetCell(cell);
        if ( cell->IsLinear() )
        {
          if (cell->GetCellDimension() == 3)
          {
            int numFaces = cell->GetNumberOfFaces();
            for (j=0; j < numFaces; j++)
            {
              face = cell->GetFace(j);
              numFacePts = face->GetNumberOfPoints();
              if (numFacePts == 4)
              {
                this->InsertQuadInHash(face->PointIds->GetId(0),
                                       face->PointIds->GetId(1),
                                       face->PointIds->GetId(2),
                                       face->PointIds->GetId(3), cellId);
              }
              else if (numFacePts == 3)
              {
                this->InsertTriInHash(face->PointIds->GetId(0),
                                      face->PointIds->GetId(1),
                                      face->PointIds->GetId(2), cellId);
              }
              else
              {
                this->InsertPolygonInHash(face->PointIds->GetPointer(0),
                                          face->PointIds->GetNumberOfIds(),
                                          cellId);
              }
            } // for all cell faces
          } // if 3D
          else
          {
            vtkDebugMacro("Missing cell type.");
          }
        } // a linear cell type
        else //process nonlinear cells via triangulation
        {
          if ( cell->GetCellDimension() == 1 )
          {
            cell->Triangulate(0,pts,coords);
            for (i=0; i < pts->GetNumberOfIds(); i+=2)
            {
              newLines->InsertNextCell(2);
              inPtId = pts->GetId(i);
              this->RecordOrigCellId(this->NumberOfNewCells, cellId);
              outputCD->CopyData( cd, cellId, this->NumberOfNewCells++ );
              outPtId = this->GetOutputPointId(inPtId, input, newPts, outputPD);
              newLines->InsertCellPoint(outPtId);
              inPtId = pts->GetId(i+1);
              outPtId = this->GetOutputPointId(inPtId, input, newPts, outputPD);
              newLines->InsertCellPoint(outPtId);
            }
          }
          else if ( cell->GetCellDimension() == 2 )
          {
            vtkWarningMacro(<< "2-D nonlinear cells must be processed with all other 2-D cells.");
          }
          else //3D nonlinear cell
          {
            vtkIdList *cellIds = vtkIdList::New();
            int numFaces = cell->GetNumberOfFaces();
            for (j=0; j < numFaces; j++)
            {
              face = cell->GetFace(j);
              input->GetCellNeighbors(cellId, face->PointIds, cellIds);
              if ( cellIds->GetNumberOfIds() <= 0)
              {
                // FIXME: Face could not be consistent. vtkOrderedTriangulator is a better option
                if (this->NonlinearSubdivisionLevel >= 1)
                {
                  // TODO: Handle NonlinearSubdivisionLevel > 1 correctly.
                  face->Triangulate(0,pts,coords);
                  for (i=0; i < pts->GetNumberOfIds(); i+=3)
                  {
                    this->InsertTriInHash(pts->GetId(i), pts->GetId(i+1),
                                          pts->GetId(i+2), cellId);
                  }
                }
                else
                {
                  switch (face->GetCellType())
                  {
                    case VTK_QUADRATIC_TRIANGLE:
                      this->InsertTriInHash(face->PointIds->GetId(0),
                                            face->PointIds->GetId(1),
                                            face->PointIds->GetId(2), cellId);
                      break;
                    case VTK_QUADRATIC_QUAD:
                    case VTK_BIQUADRATIC_QUAD:
                    case VTK_QUADRATIC_LINEAR_QUAD:
                      this->InsertQuadInHash(face->PointIds->GetId(0),
                                             face->PointIds->GetId(1),
                                             face->PointIds->GetId(2),
                                             face->PointIds->GetId(3), cellId);
                      break;
                    default:
                      vtkWarningMacro(<< "Encountered unknown nonlinear face.");
                      break;
                  } // switch cell type
                } // subdivision level
              } // cell has ids
            } // for faces
            cellIds->Delete();
          } //3d cell
        } //nonlinear cell
      } // default switch case
    } // switch(cellType)
  } // for all cells.

  // It would be possible to add these (except for polygons with 5+ sides)
  // to the hashes.  Alternatively, the higher order 2d cells could be handled
  // in the following loop.

  // Now insert 2DCells.  Because of poly datas (cell data) ordering,
  // the 2D cells have to come after points and lines.
  for(cellIter->InitTraversal();
      !cellIter->IsDoneWithTraversal() && !abort && flag2D;
      cellIter->GoToNextCell())
  {
    vtkIdType cellId = cellIter->GetCellId();
    cellType = cellIter->GetCellType();
    numCellPts = cellIter->GetNumberOfPoints();

    // If we have a quadratic face and our subdivision level is zero, just treat
    // it as a linear cell.  This should work so long as the first points of the
    // quadratic cell correspond to all those of the equivalent linear cell
    // (which all the current definitions do).
    if (this->NonlinearSubdivisionLevel < 1)
    {
      switch (cellType)
      {
        case VTK_QUADRATIC_TRIANGLE:
          cellType = VTK_TRIANGLE;  numCellPts = 3;
          break;
        case VTK_QUADRATIC_QUAD:
        case VTK_BIQUADRATIC_QUAD:
        case VTK_QUADRATIC_LINEAR_QUAD:
          cellType = VTK_POLYGON;  numCellPts = 4;
          break;
      }
    }

    // A couple of common cases to see if things go faster.
    if (cellType == VTK_PIXEL)
    { // Do we really want to insert the 2D cells into a hash?
      pointIdList = cellIter->GetPointIds();
      ids = pointIdList->GetPointer(0);
      pts->Reset();
      pts->InsertId(0, this->GetOutputPointId(ids[0], input, newPts, outputPD));
      pts->InsertId(1, this->GetOutputPointId(ids[1], input, newPts, outputPD));
      pts->InsertId(2, this->GetOutputPointId(ids[3], input, newPts, outputPD));
      pts->InsertId(3, this->GetOutputPointId(ids[2], input, newPts, outputPD));
      newPolys->InsertNextCell(pts);
      this->RecordOrigCellId(this->NumberOfNewCells, cellId);
      outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
    }
    else if (cellType == VTK_POLYGON || cellType == VTK_TRIANGLE || cellType == VTK_QUAD)
    {
      pointIdList = cellIter->GetPointIds();
      ids = pointIdList->GetPointer(0);
      pts->Reset();
      for ( i=0; i < numCellPts; i++)
      {
        inPtId = ids[i];
        outPtId = this->GetOutputPointId(inPtId, input, newPts, outputPD);
        pts->InsertId(i, outPtId);
      }
      newPolys->InsertNextCell(pts);
      this->RecordOrigCellId(this->NumberOfNewCells, cellId);
      outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
    }
    else if (cellType == VTK_TRIANGLE_STRIP)
    {
      pointIdList = cellIter->GetPointIds();
      ids = pointIdList->GetPointer(0);
      // Change strips to triangles so we do not have to worry about order.
      int toggle = 0;
      vtkIdType ptIds[3];
      // This check is not really necessary.  It was put here because of another (now fixed) bug.
      if (numCellPts > 1)
      {
        ptIds[0] = this->GetOutputPointId(ids[0], input, newPts, outputPD);
        ptIds[1] = this->GetOutputPointId(ids[1], input, newPts, outputPD);
        for (i = 2; i < numCellPts; ++i)
        {
          ptIds[2] = this->GetOutputPointId(ids[i], input, newPts, outputPD);
          newPolys->InsertNextCell(3, ptIds);
          this->RecordOrigCellId(this->NumberOfNewCells, cellId);
          outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
          ptIds[toggle] = ptIds[2];
          toggle = !toggle;
        }
      }
    }
    else if ( cellType == VTK_QUADRATIC_TRIANGLE
           || cellType == VTK_BIQUADRATIC_TRIANGLE
           || cellType == VTK_QUADRATIC_QUAD
           || cellType == VTK_BIQUADRATIC_QUAD
           || cellType == VTK_QUADRATIC_LINEAR_QUAD
           || cellType == VTK_QUADRATIC_POLYGON)
    {
      // If all of the cell points are duplicate (boundary), do not
      // extract as a surface cell.
      // If one of the points is hidden (meaning invalid), do not
      // extract surface cell.
      bool allGhosts = true;
      bool oneHidden = false;
      pointIdList = cellIter->GetPointIds();
      vtkIdType nIds = pointIdList->GetNumberOfIds();
      if (!ghosts)
      {
        allGhosts = false;
      }
      else
      {
        for ( i=0; i < nIds; i++ )
        {
          unsigned char val = ghosts->GetValue(pointIdList->GetId(i));
          if (!(val & vtkDataSetAttributes::DUPLICATEPOINT))
          {
            allGhosts = false;
          }
          if (val & vtkDataSetAttributes::HIDDENPOINT)
          {
            oneHidden = true;
            break;
          }
        }
      }
      // If all points of the polygon are ghosts, we throw it away.
      if (allGhosts || oneHidden)
      {
        continue;
      }

      // Note: we should not be here if this->NonlinearSubdivisionLevel is less
      // than 1.  See the check above.
      cellIter->GetCell(cell);
      cell->Triangulate( 0, pts, coords );
      // Copy the level 1 subdivision points (which also exist in the input and
      // can therefore just be copied over.  Note that the output of Triangulate
      // records triangles in pts where each 3 points defines a triangle.  We
      // will keep this invariant and also keep the same invariant in
      // parametericCoords and outPts later.
      outPts->Reset();
      for ( i=0; i < pts->GetNumberOfIds(); i++ )
      {
        vtkIdType op;
        op = this->GetOutputPointId(pts->GetId(i), input, newPts, outputPD);
        outPts->InsertNextId(op);
      }
      // Do any further subdivision if necessary.
        double *pc = cell->GetParametricCoords();
      if (this->NonlinearSubdivisionLevel > 1 && pc)
      {
        // We are going to need parametric coordinates to further subdivide.
        parametricCoords->Reset();
        parametricCoords->SetNumberOfComponents(3);
        for (i = 0; i < pts->GetNumberOfIds(); i++)
        {
          vtkIdType ptId = pts->GetId(i);
          vtkIdType cellPtId;
          for (cellPtId = 0; cell->GetPointId(cellPtId) != ptId; cellPtId++)
          {
          }
          parametricCoords->InsertNextTypedTuple(pc + 3*cellPtId);
        }
        // Subdivide these triangles as many more times as necessary.  Remember
        // that we have already done the first subdivision.
        for (j = 1; j < this->NonlinearSubdivisionLevel; j++)
        {
          parametricCoords2->Reset();
          parametricCoords2->SetNumberOfComponents(3);
          outPts2->Reset();
          // Each triangle will be split into 4 triangles.
          for (i = 0; i < outPts->GetNumberOfIds(); i += 3)
          {
            // Hold the input point ids and parametric coordinates.  First 3
            // indices are the original points.  Second three are the midpoints
            // in the edges (0,1), (1,2) and (2,0), respectively (see comment
            // below).
            vtkIdType inPts[6];
            double inParamCoords[6][3];
            int k;
            for (k = 0; k < 3; k++)
            {
              inPts[k] = outPts->GetId(i+k);
              parametricCoords->GetTypedTuple(i+k, inParamCoords[k]);
            }
            for (k = 3; k < 6; k++)
            {
              int pt1 = k-3;
              int pt2 = (pt1 < 2) ? (pt1 + 1) : 0;
              inParamCoords[k][0] = 0.5*(inParamCoords[pt1][0] + inParamCoords[pt2][0]);
              inParamCoords[k][1] = 0.5*(inParamCoords[pt1][1] + inParamCoords[pt2][1]);
              inParamCoords[k][2] = 0.5*(inParamCoords[pt1][2] + inParamCoords[pt2][2]);
              inPts[k] = GetInterpolatedPointId(inPts[pt1], inPts[pt2],
                                                input, cell,
                                                inParamCoords[k], newPts,
                                                outputPD);
            }
            //       * 0
            //      / \        Use the 6 points recorded
            //     /   \       in inPts and inParamCoords
            //  3 *-----* 5    to create the 4 triangles
            //   / \   / \     shown here.
            //  /   \ /   \    .
            // *-----*-----*
            // 1     4     2
            const int subtriangles[12] = {0,3,5,   3,1,4,   3,4,5,   5,4,2};
            for (k = 0; k < 12; k++)
            {
              int localId = subtriangles[k];
              outPts2->InsertNextId(inPts[localId]);
              parametricCoords2->InsertNextTypedTuple(inParamCoords[localId]);
            }
          } // Iterate over triangles
          // Now that we have recorded the subdivided triangles in outPts2 and
          // parametricCoords2, swap them with outPts and parametricCoords to
          // make them the current ones.
          std::swap(outPts, outPts2);
          std::swap(parametricCoords, parametricCoords2);
        } // Iterate over subdivision levels
      } // If further subdivision

      // Now that we have done all the subdivisions and created all of the
      // points, record the triangles.
      for (i = 0; i < outPts->GetNumberOfIds(); i += 3)
      {
        newPolys->InsertNextCell(3, outPts->GetPointer(i));
        this->RecordOrigCellId(this->NumberOfNewCells, cellId);
        outputCD->CopyData(cd, cellId, this->NumberOfNewCells++);
      }
    }
  } // for all cells.


  // Now transfer geometry from hash to output (only triangles and quads).
  this->InitQuadHashTraversal();
  while ( (q = this->GetNextVisibleQuadFromHash()) )
  {
    // If all of the cell points are duplicate (boundary), do not
    // extract as a surface cell.
    // If one of the points is hidden (meaning invalid), do not
    // extract surface cell.
    bool allGhosts = true;
    bool oneHidden = false;
    if (!ghosts)
    {
      allGhosts = false;
    }
    // handle all polys
    for (i = 0; i < q->numPts; i++)
    {
      if (ghosts)
      {
        unsigned char val = ghosts->GetValue(q->ptArray[i]);
        if (!(val & vtkDataSetAttributes::DUPLICATEPOINT))
        {
          allGhosts = false;
        }
        if (val & vtkDataSetAttributes::HIDDENPOINT)
        {
          oneHidden = true;
        }
      }

      q->ptArray[i] = this->GetOutputPointId(q->ptArray[i], input, newPts, outputPD);
    }

    // If all points of the polygon are ghosts, we throw it away.
    if (allGhosts || oneHidden)
    {
      continue;
    }
    newPolys->InsertNextCell(q->numPts, q->ptArray);
    this->RecordOrigCellId(this->NumberOfNewCells, q);
    outputCD->CopyData(inputCD, q->SourceId, this->NumberOfNewCells++);
  }

  if (this->PassThroughCellIds)
  {
    outputCD->AddArray(this->OriginalCellIds);
  }
  if (this->PassThroughPointIds)
  {
    outputPD->AddArray(this->OriginalPointIds);
  }

  // Update ourselves and release memory
  //
  cell->Delete();
  coords->Delete();
  pts->Delete();
  parametricCoords->Delete();
  parametricCoords2->Delete();
  outPts->Delete();
  outPts2->Delete();

  output->SetPoints(newPts);
  newPts->Delete();
  output->SetPolys(newPolys);
  newPolys->Delete();
  if (newVerts->GetNumberOfCells() > 0)
  {
    output->SetVerts(newVerts);
  }
  newVerts->Delete();
  newVerts = NULL;
  if (newLines->GetNumberOfCells() > 0)
  {
    output->SetLines(newLines);
  }
  newLines->Delete();

  //free storage
  output->Squeeze();
  if (this->OriginalCellIds != NULL)
  {
    this->OriginalCellIds->Delete();
    this->OriginalCellIds = NULL;
  }
  if (this->OriginalPointIds != NULL)
  {
    this->OriginalPointIds->Delete();
    this->OriginalPointIds = NULL;
  }

  this->DeleteQuadHash();

  return 1;
}

vtkIdType vtkMappedDataSetSurfaceFilter::EstimateNecessaryQuadHashSize(vtkUnstructuredGridBase *input)
{
    vtkSmartPointer<vtkCellIterator> cellIter =
        vtkSmartPointer<vtkCellIterator>::Take(input->NewCellIterator());

    vtkIdType maxPointId = -1;

    for (cellIter->InitTraversal(); !cellIter->IsDoneWithTraversal();
        cellIter->GoToNextCell())
    {
        vtkIdList *pointIdList = cellIter->GetPointIds();
        vtkIdType numCellPts = pointIdList->GetNumberOfIds();
        for (vtkIdType i = 0; i < numCellPts; i++)
        {
            vtkIdType pointId = pointIdList->GetId(i);
            if (pointId > maxPointId)
                maxPointId = pointId;
        }
    }

    return (maxPointId < 0 ? 0 : maxPointId);
}
