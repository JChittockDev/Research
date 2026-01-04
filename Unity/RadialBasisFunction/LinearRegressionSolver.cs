using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using MathNet.Spatial;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Factorization;
using mQuaternion = MathNet.Spatial.Euclidean.Quaternion;
using mVector3 = MathNet.Spatial.Euclidean.Vector3D;

[ExecuteInEditMode]
public partial class LinearRegressionSolver
{
    public int rbf;
    public float radius;
    public float distanceNorm;
    public SolverSpace solverSpace;

    public Matrix<float> theta;
    public Vector<float> sampleRadius;
    public Matrix<float> outScalarMatrix;
    public List<List<mQuaternion>> inQuatMatrix;

    public enum SolverSpace
    {
        Swing,
        Twist,
        SwingTwist
    };

    // Calculates the swing and twist distance between two quaternion values
    public void SwingTwistDistance(mQuaternion q1, mQuaternion q2, out double swingDistance, out double twistDistance)
    {
        // Decompose q1 and q2 into their respective swing and twist components
        mQuaternion sampleIndex1, t1, sampleIndex2, t2;
        DecomposeSwingTwist(q1, out sampleIndex1, out t1);
        DecomposeSwingTwist(q2, out sampleIndex2, out t2);
        // Calculate the distance between the swing components of q1 and q2
        swingDistance = mQuaternion.Distance(sampleIndex1, sampleIndex2);
        // Calculate the distance between the twist components of q1 and q2
        twistDistance = mQuaternion.Distance(t1, t2);
    }

    // This method takes a quaternion and decomposes it into a "swing" and "twist" component.
    public void DecomposeSwingTwist(mQuaternion rotation, out mQuaternion swingRotation, out mQuaternion twistRotation)
    {
        // Convert the input quaternion to a Unity Quaternion object
        Quaternion localRotation = new Quaternion((float)rotation.ImagX, (float)rotation.ImagY, (float)rotation.ImagZ, (float)rotation.Real);

        // Initialize two new Unity Quaternion objects for the "swing" and "twist" components
        Quaternion twist = new Quaternion();
        Quaternion swing = new Quaternion();

        // Set the "twist" quaternion to the rotation around the x-axis
        twist.x = localRotation.x;
        twist.y = 0.0f;
        twist.z = 0.0f;
        twist.w = localRotation.w;

        // Normalize the "twist" quaternion to ensure that it represents a valid rotation
        twist = twist.normalized;

        // Calculate the "swing" quaternion by subtracting the "twist" rotation from the original rotation
        swing = localRotation * Quaternion.Inverse(twist);
        twistRotation = new mQuaternion(twist.w, twist.x, twist.y, twist.z);
        swingRotation = new mQuaternion(swing.w, swing.x, swing.y, swing.z);
    }

    // Calculates the pseudoinverse of a matrix using Singular Value Decomposition (SVD)
    public Matrix<float> PseudoInverse(in Matrix<float> matrix, in float epsilon)
    {
        // Perform SVD on the input matrix
        var svd = matrix.Svd(true);
        // Extract the singular values from the SVD
        var singularValues = svd.S;
        // Find the maximum singular value
        var maxSingularValue = singularValues.AbsoluteMaximum();
        // Calculate the tolerance value based on the epsilon input and the dimensions of the matrix
        var tolerance = epsilon * Mathf.Max(matrix.ColumnCount, matrix.RowCount) * maxSingularValue;
        // Create a diagonal matrix from the reciprocal of the singular values that are greater than the tolerance
        var diagonal = svd.S.Map(s => Mathf.Abs(s) > tolerance ? 1 / s : 0);
        var diagonalMatrix = Matrix<float>.Build.Diagonal(diagonal.ToArray());
        // Calculate the pseudoinverse of the matrix by multiplying the conjugate transpose of U with the diagonal matrix and V, which is found by transposing VT
        return svd.VT.Transpose() * diagonalMatrix * svd.U.ConjugateTranspose();
    }

    public void SetFeatures(in List<List<mQuaternion>> inputQuatMatrix, in Matrix<float> outputScalarMatrix, in int rbfType, in float rad, in float regularization, in SolverSpace space)
    {
        // Assign input parameters to instance variables
        inQuatMatrix = inputQuatMatrix;
        outScalarMatrix = outputScalarMatrix;

        // Assign other input parameters to instance variables
        rbf = rbfType;
        radius = rad;
        solverSpace = space;

        // Calculate number of samples and initialize theta matrix
        int sampleCount = inputQuatMatrix.Count;
        theta = Matrix<float>.Build.Dense(0, 0);

        // Calculate total number of columns needed for m matrix
        int inputQuatCount = inputQuatMatrix[0].Count;
        int totalColumns = sampleCount * 2 * inputQuatCount;

        // Initialize m matrix and mQuat list
        Matrix<float> m = Matrix<float>.Build.Dense(sampleCount, totalColumns, 0.0f);
        List<Matrix<float>> mQuat = new List<Matrix<float>>();

        for (int mQuatIndex = 0; mQuatIndex < inputQuatCount; mQuatIndex++)
        {
            mQuat.Add(Matrix<float>.Build.Dense(sampleCount, sampleCount * 2));
        }
        sampleRadius = Vector<float>.Build.Dense(sampleCount, 1.0f);

        // Initialize sampleRadius vector and minimum falloff distance
        double minFalloff = double.MaxValue;
        double swingDistance, twistDistance;

        // For each sample, loop through all the other samples and calculate the swing and twist distances between the current samples quaternion values and subsequent sample quaternion values
        for (int sampleIndex1 = 0; sampleIndex1 < sampleCount; sampleIndex1++)
        {
            for (int sampleIndex2 = 0; sampleIndex2 < sampleCount; sampleIndex2++)
            {
                for (int inputQuatIndex = 0; inputQuatIndex < inputQuatCount; inputQuatIndex++)
                {
                    mQuaternion q1 = inputQuatMatrix[sampleIndex1][inputQuatIndex];
                    mQuaternion q2 = inputQuatMatrix[sampleIndex2][inputQuatIndex];
                    // Compute swing and twist distances between the two quaternions
                    SwingTwistDistance(q1, q2, out swingDistance, out twistDistance);

                    // Set either the swing or twist distance to 0 depending on the solver space.
                    if (solverSpace == SolverSpace.Swing)
                    {
                        twistDistance = 0.0;
                    }
                    else if (solverSpace == SolverSpace.Twist)
                    {
                        swingDistance = 0.0;
                    }
                    
                    if (swingDistance > 0.000001 && swingDistance < sampleRadius[sampleIndex1])
                    {
                        sampleRadius[sampleIndex1] = (float)swingDistance;
                    }
                    if (twistDistance > 0.000001 && twistDistance < sampleRadius[sampleIndex1])
                    {
                        sampleRadius[sampleIndex1] = (float)twistDistance;
                    }

                    // Add swing and twist distances to mQuat matrix
                    mQuat[inputQuatIndex][sampleIndex1, sampleIndex2 * 2] = (float)swingDistance;
                    mQuat[inputQuatIndex][sampleIndex1, sampleIndex2 * 2 + 1] = (float)twistDistance;
                }
            }
        }

        // Apply radial basis function to mQuat matrix and add to m matrix
        for (int mQuatIndex = 0; mQuatIndex < mQuat.Count; mQuatIndex++)
        {
            for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
            {
                // Get the submatrix for the distances between the ith quaternion and all other quaternions
                var subMatrix = mQuat[mQuatIndex].SubMatrix(0, sampleCount, sampleIndex * 2, 2);
                // Transform the distances using radial basis functions
                ApplyRbf(ref subMatrix, rbf, sampleRadius[sampleIndex] * radius);
                // Replace the original values with the new RBF weighted ones
                mQuat[mQuatIndex].SetSubMatrix(0, sampleCount, sampleIndex * 2, 2, subMatrix);

            }
            // Set the submatrix for the current input quaternion in the input matrix 'm'
            m.SetSubMatrix(0, 0, sampleCount, mQuat[mQuatIndex].ColumnCount * mQuatIndex, 0, mQuat[mQuatIndex].ColumnCount, mQuat[mQuatIndex]);
        }

        // Create an identity matrix for the output
        Matrix<float> outputMatrix = Matrix<float>.Build.DenseIdentity(sampleCount);

        // Create a diagonal matrix with the regularization parameter
        Matrix<float> diagonalMatrix = Matrix<float>.Build.Dense(totalColumns, totalColumns);
        Vector<float> regularizationVector = Vector<float>.Build.Dense(totalColumns, regularization);
        diagonalMatrix.SetDiagonal(regularizationVector);

        // Calculate the pseudo-inverse of the transpose of the input matrix 'm'
        Matrix<float> tm = m.Transpose();
        Matrix<float> mat = PseudoInverse(tm * m + diagonalMatrix, 1.19209e-07f) * tm;

        // Calculate the output matrix by multiplying the pseudo-inverse by the output identity matrix and then transposing it
        theta = (mat * outputMatrix).Transpose();
    }

    // This method takes a List of mQuaternions as input and returns a Vector of floats as output
    public Vector<float> Solve(in List<mQuaternion> inputQuats) 
    {
        // Get the number of samples in the input and create a Vector to store input distances
        int sampleCount = inQuatMatrix.Count;
        Vector<float> inputDistance = Vector<float>.Build.Dense(theta.ColumnCount);

        // Initialize variables
        int idx = 0;
        int inputQuatCount = inQuatMatrix[0].Count;
        double swingDistance, twistDistance;

        // For each input quaternion, loop through the samples and find the swing and twist distances between the given input and subsequent sample quaternions
        for (int inputQuatIndex1 = 0; inputQuatIndex1 < inputQuatCount; inputQuatIndex1++)
        {
            mQuaternion inputQuaternion = inputQuats[inputQuatIndex1];
            for (int sampleIndex1 = 0; sampleIndex1 < sampleCount; sampleIndex1++)
            {
                int startIdx = idx;
                for (int inputQuatIndex2 = 0; inputQuatIndex2 < inputQuatCount; inputQuatIndex2++)
                {
                    mQuaternion sampleInputQuaternion = inQuatMatrix[sampleIndex1][inputQuatIndex2];

                    // Compute swing and twist distances between the two quaternions
                    SwingTwistDistance(inputQuaternion, sampleInputQuaternion, out swingDistance, out twistDistance);

                    // Set either the swing or twist distance to 0 depending on the solver space.
                    if (solverSpace == SolverSpace.Swing)
                    {
                        twistDistance = 0.0;
                    }
                    else if (solverSpace == SolverSpace.Twist)
                    {
                        swingDistance = 0.0;
                    }

                    // Store the swing and twist distances in the input distance vector.
                    inputDistance[idx++] = (float)swingDistance;
                    inputDistance[idx++] = (float)twistDistance;
                }

                // Get the block of input distances and apply the radial basis function to the block.
                var block = inputDistance.SubVector(startIdx, inputQuatCount * 2);
                Matrix<float> blockDistanceMatrix = block.ToRowMatrix();
                ApplyRbf(ref blockDistanceMatrix, rbf, sampleRadius[sampleIndex1]);
                // Replace the original values with the RBF weighted ones
                inputDistance.SetSubVector(startIdx, inputQuatCount * 2, blockDistanceMatrix.Row(0));
            }
        }

        // Get the number of output values and create a Vector to store them
        int outputCount = outScalarMatrix.ColumnCount;
        Vector<float> outputValues = Vector<float>.Build.Dense(outputCount);

        // Multiply the input distances by the theta values to get the weights
        Vector<float> weights = theta * inputDistance;

        // Loop through each output value and compute its weighted sum
        for (int outputValueIndex = 0; outputValueIndex < outputCount; outputValueIndex++)
        {
            outputValues[outputValueIndex] = weights.DotProduct(outScalarMatrix.Column(outputValueIndex));
        }

        return outputValues;
    }
}
