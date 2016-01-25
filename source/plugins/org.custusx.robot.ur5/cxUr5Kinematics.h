#ifndef UR5KINEMATICS_H
#define UR5KINEMATICS_H

#include "org_custusx_robot_ur5_Export.h"
#include "cxVector3D.h"
#include "cxTransform3D.h"



namespace cx
{
/**
 * Implementation of UR5 Kinematics.
 *
 * \ingroup org_custusx_robot_ur5
 *
 * \author Andreas Østvik
 */

#define d1 0.089159
#define a2 -0.42500
#define a3 -0.39225
#define d4 0.10915
#define d5 0.09465
#define d6 0.0823

class org_custusx_robot_ur5_EXPORT Ur5Kinematics
{
public:
    Transform3D forward(Eigen::RowVectorXd jointConfiguration);
    Eigen::MatrixXd jacobian(Eigen::RowVectorXd jointConfiguration);

    Eigen::RowVectorXd inverseJ(Transform3D desiredPose, Eigen::RowVectorXd guessedJointConfiguration);

    Transform3D poseToMatrix(Eigen::RowVectorXd poseConfiguration);

    Vector3D T2transl(Eigen::MatrixXd T);
    Vector3D T2transl(Transform3D T);
    Vector3D T2rangles(Transform3D T);

    Eigen::MatrixXd getRotation(Transform3D pose);
    Eigen::RowVectorXd errorVector(Transform3D desiredPose, Transform3D currentPose);
    Eigen::MatrixXd pseudoInverse(Eigen::MatrixXd matrix);

    Transform3D T01(Eigen::RowVectorXd jointConfiguration);
    Transform3D T02(Eigen::RowVectorXd jointConfiguration);
    Transform3D T03(Eigen::RowVectorXd jointConfiguration);
    Transform3D T04(Eigen::RowVectorXd jointConfiguration);
    Transform3D T05(Eigen::RowVectorXd jointConfiguration);

private:
};

} // cx

#endif // UR5KINEMATICS_H
