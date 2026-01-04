using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Factorization;
using MathNet.Spatial;

using mQuaternion = MathNet.Spatial.Euclidean.Quaternion;

public partial class LinearRegressionSolver
{
    // Define a structure for the Gaussian radial basis function(RBF)
    public struct Gaussian
    {
        public float r; // Define a public float variable for the radius of the RBF
        // Define a constructor for the Gaussian RBF that takes a float radius argument
        public Gaussian(float radius)
        {
            const float kFalloff = 0.4f;
            r = radius > 0.0f ? radius : 0.001f; // Set the radius variable to the input radius, or 0.001f if radius is less than or equal to 0
            r *= kFalloff; // Apply a modification to the radius
        }
        // Define a function for evaluating the Gaussian RBF at a given value x
        public float Eval(float x)
        {
            return (float)Math.Exp(-(x * x) / (2.0f * r * r)); // Calculate the Gaussian RBF value at x using the formula (e^(-x^2/(2*r^2)))
        }
    }

    // Define a structure for the Thin Plate radial basis function (RBF)
    public struct ThinPlate
    {
        public float r; // Define a public float variable for the radius of the RBF
        // Define a constructor for the Thin Plate RBF that takes a float radius argument
        public ThinPlate(float radius)
        {
            r = radius > 0.0f ? radius : 0.001f; // Set the radius variable to the input radius, or 0.001f if radius is less than or equal to 0
        }
        // Define a function for evaluating the Thin Plate RBF at a given value x
        public float Eval(float x)
        {
            float v = x / r; // Calculate the ratio x/r
            return v > 0.0f ? v * v * (float)Math.Log(v) : v; // Calculate the Thin Plate RBF value at x using the formula (|x|*|x|*ln(|x|) if x is greater than 0, or x otherwise)
        }
    }

    // Define a structure for the Multi-Quadratic Biharmonic radial basis function (RBF)
    public struct MultiQuadraticBiharmonic
    {
        public float r; // Define a public float variable for the radius of the RBF
        // Define a constructor for the Multi-Quadratic Biharmonic RBF that takes a float radius argument
        public MultiQuadraticBiharmonic(float radius)
        {
            r = radius; // Set the radius variable to the input radius
        }
        // Define a function for evaluating the Multi-Quadratic Biharmonic RBF at a given value x
        public float Eval(float x)
        {
            return (float)Math.Sqrt((x * x) + (r * r)); // Calculate the Multi-Quadratic Biharmonic RBF value at x using the formula sqrt(x^2 + r^2)
        }
    }


    // Define a struct that represents the inverse multi-quadratic biharmonic radial basis function.
    public struct InverseMultiQuadraticBiharmonic
    {
        // The radius parameter of the RBF.
        public float r;
        // Constructor that sets the radius parameter.
        public InverseMultiQuadraticBiharmonic(float radius)
        {
            r = radius;
        }
        // Method that computes the RBF value for a given input x.
        public float Eval(float x)
        {
            // Return 1 divided by the square root of the sum of x squared and the radius squared.
            return 1.0f / (float)Math.Sqrt((x * x) + (r * r));
        }
    }

    // Define a struct that represents the Beckert-Wendland C2 basis radial basis function.
    public struct BeckertWendlandC2Basis
    {
        // The radius parameter of the RBF.
        public float r;
        // Constructor that sets the radius parameter, ensuring that it is not zero.
        public BeckertWendlandC2Basis(float radius)
        {
            r = radius > 0.0f ? radius : 0.001f;
        }
        // Method that computes the RBF value for a given input x.
        public float Eval(float x)
        {
            // Compute the ratio of x to the radius.
            float v = x / r;
            // Compute the first term of the RBF.
            float first = (1.0f - v > 0.0f) ? (float)Math.Pow(1.0f - v, 4) : 0.0f;
            // Compute the second term of the RBF.
            float second = 4.0f * v + 1.0f;
            // Return the product of the two terms.
            return first * second;
        }
    }

    // Define a method that applies an RBF to a matrix of floating-point values.
    public void ApplyRbf(ref Matrix<float> m, int rbf, float radius)
    {
        // Use a switch statement to select the appropriate RBF based on the provided index.
        switch (rbf)
        {
            // Case 0: do nothing.
            case 0:
                break;
            // Case 1: apply the Gaussian RBF.
            case 1:
                m.MapInplace(new Gaussian(radius).Eval);
                break;
            // Case 2: apply the Thin Plate RBF.
            case 2:
                m.MapInplace(new ThinPlate(radius).Eval);
                break;
            // Case 3: apply the Multi-Quadratic Biharmonic RBF.
            case 3:
                m.MapInplace(new MultiQuadraticBiharmonic(radius).Eval);
                break;
            // Case 4: apply the Inverse Multi-Quadratic Biharmonic RBF.
            case 4:
                m.MapInplace(new InverseMultiQuadraticBiharmonic(radius).Eval);
                break;
            // Case 5: apply the Beckert-Wendland C2 Basis RBF.
            case 5:
                m.MapInplace(new BeckertWendlandC2Basis(radius).Eval);
                break;
        }
    }
}
