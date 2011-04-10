#ifndef SSCVECTOR3D_H_
#define SSCVECTOR3D_H_

//#include "boost/array.hpp"
class QString;
#include <QStringList>

#define EIGEN_PLAINOBJECTBASE_PLUGIN "sscPlainObjectEigenAddons.h"
#define EIGEN_MATRIXBASE_PLUGIN "sscMatrixBaseEigenAddons.h"

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

// --------------------------------------------------------
namespace ssc
{
//namespace utils
//{
// --------------------------------------------------------

bool similar(double a, double b, double tol=1.0E-6);    ///< check for equality with a tolerance: |b-a|<tol

/**Use math library Eigen for the the ssc type Vector3D
 *
 */
typedef Eigen::Vector3d Vector3D;

///** Representation of a point or vector in 3D.
// *  Supports standard mathematical operators.
// */
//class Vector3D : public boost::array<double,3>
//{
//public:
//	Vector3D();
//	Vector3D(double x, double y, double z);
//	explicit Vector3D(const double* data); ///< construct vector from a c-style array of size 3.
//	explicit Vector3D(const int* data);    ///< construct vector from a c-style array of size 3.
//  static Vector3D fromString(const QString& text); ///< construct a vector from a string containing 3 whitespace-separated numbers
//
//	Vector3D& operator+=(const Vector3D& b);
//	Vector3D& operator-=(const Vector3D& b);
//	Vector3D& operator*=(double b);
//	Vector3D& operator/=(double b);
//	Vector3D operator-() const;
//
//	Vector3D normal() const;     ///< return the vector with length 1.
//	double length() const;       ///< return cartesian length of vector.
//};
//// --------------------------------------------------------
//
//Vector3D operator+(const Vector3D& a, const Vector3D& b);
//Vector3D operator-(const Vector3D& a, const Vector3D& b);
//Vector3D operator*(double a, const Vector3D& b);
//Vector3D operator*(const Vector3D& a, double b);
//Vector3D operator/(const Vector3D& a, double b);
Vector3D multiply_elems(const Vector3D& a, const Vector3D& b);          ///< perform element-wise multiplication of a and b.
Vector3D divide_elems(const Vector3D& a, const Vector3D& b);            ///< perform element-wise division of a and b.
Vector3D cross(const Vector3D& a, const Vector3D& b);                   ///< compute cross product of a and b.
double dot(const Vector3D& a, const Vector3D& b);                       ///< compute inner product (or dot product) of a and b.
bool similar(const Vector3D& a, const Vector3D& b, double tol=1.0E-6);  ///< check for equality with a tolerance: |b-a|<tol
//std::ostream& operator<<(std::ostream& s, const Vector3D& data);
Vector3D unitVector(double thetaXY, double thetaZ=0.0);                     ///< compute a unit vector given angles xy in the xy plane and z meaning the elevation from the xy plane.
Vector3D round(const Vector3D& a);
Vector3D ceil(const Vector3D& a);
//// --------------------------------------------------------
////} // namespace utils
////using namespace utils;
} // namespace ssc
// --------------------------------------------------------

#endif /*SSCVECTOR3D_H_*/
