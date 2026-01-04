using System;
using System.Collections.Generic;
using UnityEngine;

namespace SoulMachines
{
    [ExecuteInEditMode]
    public class Twist : MonoBehaviour
    {
        public float theta = 0.5f;
        public int axis = 0;

        public GameObject rootJoint;
        public GameObject parentJoint;
        public GameObject endJoint;

        private Matrix matrix = new Matrix();
        private Quaternion inputRotation;

        void Awake()
        {
            if (parentJoint != null)
            {
                inputRotation = parentJoint.transform.localRotation;
            }
        }

        void Update()
        {
            if (rootJoint != null || parentJoint != null || endJoint != null)
            {
                parentJoint.transform.localRotation = inputRotation;
                Matrix4x4 rootJointMatrix = matrix.GetMatrix(rootJoint.transform, "world");
                Matrix4x4 parentJointWorldMatrix = matrix.GetMatrix(parentJoint.transform, "world");
                Matrix4x4 endJointWorldMatrix = matrix.GetMatrix(endJoint.transform, "world");
                Matrix4x4 inverseParentMatrix = parentJointWorldMatrix.inverse;
                parentJoint.transform.localRotation = calculateTwist(rootJointMatrix, endJointWorldMatrix, inverseParentMatrix, theta, axis);
            }
            else
            {
                throw new Exception("All joints need to be connected in order for this script to function");
            }
        }

        public Quaternion calculateTwist(in Matrix4x4 rootMatrix, in Matrix4x4 endMatrix, 
                                            in Matrix4x4 parentMatrix, float theta, int axis)
        {
            Quaternion rootQuat = rootMatrix.rotation;
            Quaternion endQuat = endMatrix.rotation;
            Quaternion parentQuat = parentMatrix.rotation;

            Quaternion slerpedQuat = Quaternion.Slerp(endQuat, rootQuat, theta);
            Quaternion twistQuat = parentQuat * slerpedQuat;
            if (axis == 0)
            {
                twistQuat.Set(twistQuat.x, 0, 0, twistQuat.w);
            }
            else if (axis == 1)
            {
                twistQuat.Set(0, twistQuat.y, 0, twistQuat.w);
            }
            else
            {
                twistQuat.Set(0, 0, twistQuat.z, twistQuat.w);
            }
            return twistQuat;
        }

    }
}

