using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using MathNet.Spatial;
using MathNet.Numerics.LinearAlgebra;
using MathNet.Numerics.LinearAlgebra.Factorization;
using mQuaternion = MathNet.Spatial.Euclidean.Quaternion;

[ExecuteInEditMode]
public class RBF : MonoBehaviour
{
    public int rbf;
    public float radius;
    public int inputCount;
    public int outputCount;
    public int outputMode;
    public float regularization;

    SkinnedMeshRenderer skinnedMeshRenderer;
    List<mQuaternion> inputQuaternionValues = new List<mQuaternion>();
    List<mQuaternion> inputRestQuaternionValues = new List<mQuaternion>();
    Vector<float> neutralScalarValues = Vector<float>.Build.Dense(0);
    Dictionary<string, int> boneMap = new Dictionary<string, int>();
    Dictionary<string, int> shapeMap = new Dictionary<string, int>();
    List<LinearRegressionSolver> solvers = new List<LinearRegressionSolver>();
    List<Sample> samples = new List<Sample>();

    public class Sample
    {
        public float radius;
        public int rotationType;
        public List<mQuaternion> inQuaternionValues;
        public Vector<float> outScalarValues;
    };

    // This should be replaced with a serialized json file
    void SetSamples(ref List<Sample> samples)
    {
        Sample sample1 = new Sample();
        sample1.inQuaternionValues = new List<mQuaternion>();
        sample1.inQuaternionValues.Add(new mQuaternion(0.9999167, 1.928788E-05, 0.00284244, 0.01259599));
        float[] sample1ScalarValues = { 0.0f, 0.0f, 0.0f, 0.0f };
        sample1.outScalarValues = Vector<float>.Build.DenseOfArray(sample1ScalarValues);
        sample1.rotationType = 0;
        samples.Add(sample1);

        Sample sample2 = new Sample();
        sample2.inQuaternionValues = new List<mQuaternion>();
        sample2.inQuaternionValues.Add(new mQuaternion(0.7578869, 0.001842668, 0.002167369, 0.6523799));
        float[] sample2ScalarValues = { 0.0f, 0.0f, 1.0f, 0.0f };
        sample2.outScalarValues = Vector<float>.Build.DenseOfArray(sample2ScalarValues);
        sample2.rotationType = 0;
        samples.Add(sample2);

        Sample sample3 = new Sample();
        sample3.inQuaternionValues = new List<mQuaternion>();
        sample3.inQuaternionValues.Add(new mQuaternion(0.7740747, -0.001814465, 0.002191034, -0.633088));
        float[] sample3ScalarValues = { 0.0f, 0.0f, 0.0f, 1.0f };
        sample3.outScalarValues = Vector<float>.Build.DenseOfArray(sample3ScalarValues);
        sample3.rotationType = 0;
        samples.Add(sample3);

        Sample sample4 = new Sample();
        sample4.inQuaternionValues = new List<mQuaternion>();
        sample4.inQuaternionValues.Add(new mQuaternion(0.7641519, 0.008108144, 0.6449135, 0.009634265));
        float[] sample4ScalarValues = { 1.0f, 0.0f, 0.0f, 0.0f };
        sample4.outScalarValues = Vector<float>.Build.DenseOfArray(sample4ScalarValues);
        sample4.rotationType = 0;
        samples.Add(sample4);

        Sample sample5 = new Sample();
        sample5.inQuaternionValues = new List<mQuaternion>();
        sample5.inQuaternionValues.Add(new mQuaternion(0.7678095, -0.008079933, -0.6405545, 0.009657936));
        float[] sample5ScalarValues = { 0.0f, 1.0f, 0.0f, 0.0f };
        sample5.outScalarValues = Vector<float>.Build.DenseOfArray(sample5ScalarValues);
        sample5.rotationType = 0;
        samples.Add(sample5);
    }

    // This function initializes the transform data of a given SkinnedMeshRenderer and stores the necessary information in the provided data structures.
    void InitTransforms(in SkinnedMeshRenderer smr, ref Dictionary<string, int> map, ref List<mQuaternion> restQuaternionValues, ref List<mQuaternion> inputQuaternionValues)
    {
        // Create a mapping of bone names to their corresponding indices in the SkinnedMeshRenderer's 'bones' array.
        for (int boneIndex = 0; boneIndex < smr.bones.Length; boneIndex++)
        {
            map[smr.bones[boneIndex].name] = boneIndex;
        }

        // Get the local rotation of the 'lHumerus' bone and store it as a quaternion in the 'restQuaternionValues' and 'inputQuaternionValues' lists.
        Quaternion transform = smr.bones[map["lHumerus"]].localRotation;
        restQuaternionValues.Add(new mQuaternion(transform.w, transform.x, transform.y, transform.z));
        inputQuaternionValues.Add(new mQuaternion(transform.w, transform.x, transform.y, transform.z));
    }

    // This is a private method that takes in a SkinnedMeshRenderer object named skinnedMesh and a Dictionary object that maps string keys to integer values, named shapeMap.
    private void GetBlendShapeNames(in SkinnedMeshRenderer skinnedMesh, ref Dictionary<string, int> shapeMap)
    {
        // Get the mesh from the skinned mesh renderer.
        Mesh mesh = skinnedMesh.sharedMesh;

        // Loop through each blend shape in the mesh.
        for (int blendIndex = 0; blendIndex < mesh.blendShapeCount; blendIndex++)
        {
            // Get the name of the blend shape.
            string shape = mesh.GetBlendShapeName(blendIndex);

            // Split the shape name by '.' and get the second part.
            string[] remainder = shape.Split('.');
            string shapeName = remainder[1];

            // Add the shape name and its index in the mesh's blend shape array to the shapeMap dictionary.
            shapeMap.Add(shapeName, blendIndex);
        }
    }

    // This method builds a feature matrix based on input rest quaternions
    public void BuildFeatureMatrix(in List<mQuaternion> inputRestQuats)
    {
        // Add three lists of input quaternions and output scalars for the three rotation types
        List<List<List<mQuaternion>>> inputQuats = new List<List<List<mQuaternion>>>();
        List<List<Vector<float>>> outputScalars = new List<List<Vector<float>>>();

        inputQuats.Add(new List<List<mQuaternion>>());
        inputQuats.Add(new List<List<mQuaternion>>());
        inputQuats.Add(new List<List<mQuaternion>>());

        outputScalars.Add(new List<Vector<float>>());
        outputScalars.Add(new List<Vector<float>>());
        outputScalars.Add(new List<Vector<float>>());

        // Loop through each sample in the list of samples
        for (int sampleIndex = 0; sampleIndex < samples.Count; sampleIndex++)
        {
            // Get the rotation type of the sample
            int rotationType = samples[sampleIndex].rotationType;

            // Create a list of quaternions based on the inQuaternionValues of the sample and input rest quaternions
            List<mQuaternion> quats = new List<mQuaternion>();
            for (int inputIndex = 0; inputIndex < inputCount; inputIndex++)
            {
                quats.Add(samples[sampleIndex].inQuaternionValues[inputIndex] * inputRestQuats[inputIndex].Inversed);
            }

            // Add the list of quaternions and scalar values to the appropriate lists
            inputQuats[rotationType].Add(quats);
            outputScalars[rotationType].Add(samples[sampleIndex].outScalarValues);
        }

        // Loop through each rotation type (Swing, Twist, SwingTwist)
        for (int rotationTypeIndex = 0; rotationTypeIndex < 3; rotationTypeIndex++)
        {
            // Check if there are output scalars for this rotation type
            if (outputScalars[rotationTypeIndex].Count > 0)
            {
                // If the neutralScalarValues list is empty and the output mode is 1, set the neutralScalarValues to the first output scalar values
                if (neutralScalarValues.Count == 0 && outputMode == 1)
                {
                    neutralScalarValues = outputScalars[rotationTypeIndex][0];
                }

                // Create a matrix of output scalars
                Matrix<float> outputMatrix = Matrix<float>.Build.Dense(outputScalars[rotationTypeIndex].Count, outputScalars[rotationTypeIndex][0].Count);

                int row = 0;
                // Loop through each scalar value in the outputScalars list
                foreach (var outputVector in outputScalars[rotationTypeIndex])
                {
                    // If the output mode is 0, set the row of the output matrix to the scalar values
                    if (outputMode == 0)
                    {
                        outputMatrix.SetRow(row++, outputVector);
                    }
                    // If the output mode is 1, set the row of the output matrix to the scalar values minus the neutralScalarValues
                    else
                    {
                        outputMatrix.SetRow(row++, outputVector - neutralScalarValues);
                    }
                }

                // Create a list of solver spaces for the linear regression solver
                List<LinearRegressionSolver.SolverSpace> space = new List<LinearRegressionSolver.SolverSpace>
                {LinearRegressionSolver.SolverSpace.Swing, LinearRegressionSolver.SolverSpace.Twist, LinearRegressionSolver.SolverSpace.SwingTwist};
                solvers.Add(new LinearRegressionSolver());

                // Set the features of the linear regression solver based on the input quaternions, output matrix, RBF function, radius, regularization, and solver space
                solvers[rotationTypeIndex].SetFeatures(inputQuats[rotationTypeIndex], outputMatrix, rbf, radius, regularization, space[rotationTypeIndex]);
            }
        }
    }

    // This ensure that the input Quaternion values do not contain any neutral values and are relative to the rest pose
    public void SetRest(in int count, ref List<mQuaternion> inQuatValues, in List<mQuaternion> restQuatValues)
    {
        // Loop through each element in the inQuatValues list.
        for (int quatIndex = 0; quatIndex < count; quatIndex++)
        {
            // Multiply the current element of inQuatValues by the inverse of the corresponding element in restQuatValues.
            inQuatValues[quatIndex] *= restQuatValues[quatIndex].Inversed;
        }
    }

    // This updates the current quaternion rotations store in the inputQuaternionValues variable
    void UpdateTransforms(in SkinnedMeshRenderer smr, in Dictionary<string, int> map, ref List<mQuaternion> inputQuaternionValues)
    {
        // Get the local rotation of the bone corresponding to the "lHumerus" key in the map dictionary.
        Quaternion transform = smr.bones[map["lHumerus"]].localRotation;
        inputQuaternionValues[0] = new mQuaternion(transform.w, transform.x, transform.y, transform.z);
    }

    void Start()
    {
        // Get the local skinned mesh renderer component
        skinnedMeshRenderer = GetComponent<SkinnedMeshRenderer>();
        // Initialize transoform lists
        InitTransforms(skinnedMeshRenderer, ref boneMap, ref inputRestQuaternionValues, ref inputQuaternionValues);
        // Create blendshape name map
        GetBlendShapeNames(skinnedMeshRenderer, ref shapeMap);
        // Set the current samples for regression
        SetSamples(ref samples);
        // Pass this data to the regression solver as a feature matrix
        BuildFeatureMatrix(inputRestQuaternionValues);
    }

    void Update()
    {
        // Update the transform lists with the current values
        UpdateTransforms(skinnedMeshRenderer, boneMap, ref inputQuaternionValues);
        // Ensure that the transforms do not contain in rest quaternion values
        SetRest(inputCount, ref inputQuaternionValues, inputRestQuaternionValues);
        // Initialize the output scalar matrix
        Matrix<float> outputScalars = Matrix<float>.Build.Dense(3, outputCount);

        // For each solver (swing, twist, swingtwist), solve for the RBF output weights)
        for (int solverIndex = 0; solverIndex < solvers.Count; solverIndex++)
        {
            Vector<float> scalars = solvers[solverIndex].Solve(inputQuaternionValues);
            if (scalars.Count > 0)
            {
                outputScalars.SetRow(solverIndex, scalars);
            }
        }

        // Sum the results to return the average scalar result and meet unit vector requirements
        Vector<float> outValues = outputScalars.ColumnSums();
        if (neutralScalarValues.Count > 0)
        {
            // Add the neutral values back so that the rest pose quaternion values are used
            outValues += neutralScalarValues;
        }

        // Set the resulting blendshpes 
        //Debug.Log("Ouput RBF Weights: " + "lHumerus80Y: " + Mathf.Clamp(outValues[0], 0.0f, 1.0f) + ", lHumerusn80Y: " + Mathf.Clamp(outValues[1], 0.0f, 1.0f) + ", lHumerusn80Z: " + Mathf.Clamp(outValues[2], 0.0f, 1.0f) + ", lHumerus80Z: " + Mathf.Clamp(outValues[3], 0.0f, 1.0f));
        skinnedMeshRenderer.SetBlendShapeWeight(shapeMap["lHumerusn80Y"], Mathf.Clamp(outValues[0], 0.0f, 1.0f) * 100.00f);
        skinnedMeshRenderer.SetBlendShapeWeight(shapeMap["lHumerus80Y"], Mathf.Clamp(outValues[1], 0.0f, 1.0f) * 100.00f);
        skinnedMeshRenderer.SetBlendShapeWeight(shapeMap["lHumerusn80Z"], Mathf.Clamp(outValues[2], 0.0f, 1.0f) * 100.00f);
        skinnedMeshRenderer.SetBlendShapeWeight(shapeMap["lHumerus80Z"], Mathf.Clamp(outValues[3], 0.0f, 1.0f) * 100.00f);
    }
}