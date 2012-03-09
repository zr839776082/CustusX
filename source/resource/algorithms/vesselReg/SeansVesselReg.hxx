#ifndef SeansVesselSegmentation_hxx
#define SeansVesselSegmentation_hxx

#include "sscForwardDeclarations.h"
#include "vtkForwardDeclarations.h"
#include "sscTransform3D.h"
#include "vtkSmartPointer.h"

namespace cx
{
/** Vessel - vessel registration algorithm.
 *
 * Input is two centerline representations of vessel trees.
 * Output is either a linear transform or a nonlinear transform (nonlinear not used yet).
 *
 * Original authors:
 *  Ingerid Reinertsen - algorithm.
 *  Sean (get full name) - refactored code.
 *
 */
class SeansVesselReg
{
public:
  SeansVesselReg(int lts_ratio, double stop_delta, double lambda, double sigma, bool lin_flag, int sample, int single_point_thre, bool verbose);
  ~SeansVesselReg();

  bool execute(ssc::DataPtr source, ssc::DataPtr target, QString logPath);
  ssc::Transform3D getLinearTransform();
//  ssc::Transform3D getNonLinearTransform();
  ssc::ImagePtr loadMinc(char* source_file);
  void setDebugOutput(bool on) { mt_verbose = on; }

  /**
   * Extract polydata from a image.
   * @param image
   * @param p_neighborhoodFilterThreshold
   * @param p_BoundingBox
   * @return
   */
  static vtkPolyDataPtr extractPolyData(ssc::ImagePtr image, int p_neighborhoodFilterThreshold, double p_BoundingBox[6]);
private:
  ssc::Transform3D getLinearTransform(vtkGeneralTransformPtr myConcatenation);

protected:
  bool runAlgorithm(vtkPolyDataPtr currentSourcePolyData, vtkPolyDataPtr targetPolyData, vtkGeneralTransformPtr myConcatenation);
  void printOutResults(QString fileNamePrefix, vtkGeneralTransformPtr myConcatenation);
  vtkAbstractTransformPtr linearRegistration(vtkPointsPtr sortedSourcePoints, vtkPointsPtr sortedTargetPoints, int numPoints);
  vtkAbstractTransformPtr nonLinearRegistration(vtkPointsPtr sortedSourcePoints, vtkPointsPtr sortedTargetPoints, int numPoints);
  vtkPolyDataPtr convertToPolyData(ssc::DataPtr data);
  vtkPointsPtr transformPoints(vtkPointsPtr input, vtkAbstractTransformPtr transform);
  vtkPolyDataPtr convertToPolyData(vtkPointsPtr input);
  vtkPointsPtr createSortedPoints(vtkIdListPtr sortedIDList, vtkPointsPtr unsortedPoints, int numPoints);
  vtkPolyDataPtr crop(vtkPolyDataPtr input, vtkPolyDataPtr fixed, double margin);

  void print(vtkPointsPtr points);
  void print(vtkPolyDataPtr data);

  int mt_ltsRatio;
  double mt_distanceDeltaStopThreshold;
  double mt_lambda;
  double mt_sigma;
  bool mt_doOnlyLinear;
  bool mt_doStepping;
  int mt_sampleRatio;
  int mt_singlePointThreshold;
  const int mt_maximumNumberOfIterations;
  bool mt_verbose;

  ssc::Transform3D mLinearTransformResult;

  //---------------------------------------------------------------------------
  //TODO non-linear needs to handle this!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  bool mInvertedTransform; ///< the calculated registration goes from target to source instead of source to target
  //---------------------------------------------------------------------------
};
}//namespace cx
#endif
