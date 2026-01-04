using System;
using System.Collections.Generic;
using UnityEngine;

namespace SoulMachines
{
    [ExecuteInEditMode]
    public class QuatAim : MonoBehaviour
    {
        public int upAxis = 1;
        public int aimAxis = 0;
        public bool flipUp = false;
        public bool flipAim = false;
        public bool flipCosine = true;

        public GameObject rootJoint;
        public GameObject endJoint;
        public GameObject parentJoint;

        private Matrix matrix = new Matrix();

        void Awake()
        {

        }

        void Update()
        {
            if (rootJoint != null || parentJoint != null || endJoint != null)
            {
                Matrix4x4 parentJointWorldMatrix = matrix.GetMatrix(parentJoint.transform, "world");
                Matrix4x4 endJointWorldMatrix = matrix.GetMatrix(endJoint.transform, "world");

                Vector3 rootJointTranslate = rootJoint.transform.localPosition;
                Matrix4x4 rootJointLocalMatrix = Matrix4x4.Translate(rootJointTranslate);
                Matrix4x4 rootJointWorldMatrix = rootJointLocalMatrix * parentJointWorldMatrix;

                rootJoint.transform.localRotation = calculateAim(rootJointWorldMatrix, endJointWorldMatrix, parentJointWorldMatrix, aimAxis, upAxis, flipUp, flipAim, flipCosine);
            }
            else
            {
                throw new Exception("All joints need to be connected in order for this script to function");
            }
        }

        public Quaternion calculateAim(in Matrix4x4 rootMatrix, in Matrix4x4 endMatrix, in Matrix4x4 parentMatrix, int aimAxisInt, int upAxisInt, bool flipUp, bool flipAim, bool flipCosine)
        {
            Vector3 endVector = endMatrix.GetColumn(3);
            Vector3 rootVector = rootMatrix.GetColumn(3);
            Vector3 aimAxis = parentMatrix.GetColumn(aimAxisInt);
            Vector3 upAxis = parentMatrix.GetColumn(upAxisInt);

            if (flipUp)
            {
                upAxis *= -1.0f;
            }
            
            if (flipAim)
            {
                aimAxis *= -1.0f;
            }

            aimAxis = Vector3.Normalize(aimAxis);
            Vector3 aimDirection = endVector - rootVector;
            aimDirection = Vector3.Normalize(aimDirection);
            Vector3 deltaVector = Vector3.Cross(aimAxis, aimDirection);

            if (deltaVector.magnitude == 0)
            {
                deltaVector = Vector3.Cross(upAxis, aimDirection);
            }

            float angle;
            deltaVector = Vector3.Normalize(deltaVector);
            float dot = Vector3.Dot(aimAxis, aimDirection);

            if (flipCosine)
            {
                angle = -Mathf.Acos(dot) * Mathf.Rad2Deg;
            }
            else
            {
                angle = Mathf.Acos(dot) * Mathf.Rad2Deg;
            }

            Quaternion quatRotation = Quaternion.AngleAxis(angle, deltaVector);
            //Quaternion parentQuat = parentMatrix.rotation;
            //quatRotation *= parentQuat;
            //quatRotation *= Quaternion.Inverse(parentQuat);

            return quatRotation;
        }
    }
}

