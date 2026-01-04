using System;
using System.Collections.Generic;
using UnityEngine;

namespace SoulMachines
{
    public class Matrix
    {
        public Matrix4x4 GetMatrix(in Transform transform, string space)
        {
            if (space == "local")
            {
                return Matrix4x4.TRS(transform.localPosition, transform.localRotation, new Vector3(1, 1, 1));
            }
            else if (space == "world")
            {
                return Matrix4x4.TRS(transform.position, transform.rotation, new Vector3(1, 1, 1));
            }
            else
            {
                throw new Exception("Incorrect transformation space");
            }
        }
    }
}

