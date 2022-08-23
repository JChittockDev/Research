// Copyright Joseph Chittock @2022

#include "node.h"

// Scales the vertex positions of a specific id in a floating point array
void tissueNode::vecScale(MFloatArray& a, int anr, float scale)
{
    anr *= 3;
    a[anr + 2] *= scale;
    a[anr + 1] *= scale;
    a[anr] *= scale;
}

// Performs a dot product on a specific pair of vertex positions in two arrays
float tissueNode::vecDot(MFloatArray& a, int anr, MFloatArray& b, int bnr)
{
    anr *= 3; bnr *= 3;
    return a[anr] * b[bnr] + a[anr + 1] * b[bnr + 1] + a[anr + 2] * b[bnr + 2];
}

// Stores the diffirence between two specific pairs of vertex positions in a third array
void tissueNode::vecSetDiff(MFloatArray& dst, int dnr, MFloatArray& a, int anr, MFloatArray& b, int bnr, float scale)
{
    dnr *= 3;
    anr *= 3;
    bnr *= 3;
    dst[dnr] = (a[anr] - b[bnr]) * scale;
    dst[dnr + 1] = (a[anr + 1] - b[bnr + 1]) * scale;
    dst[dnr + 2] = (a[anr + 2] - b[bnr + 2]) * scale;
}

// Copys the data from a given vertex to another within a floating array
void tissueNode::vecCopy(MFloatArray& a, int anr, const MFloatArray& b, int bnr)
{
    anr *= 3;
    bnr *= 3;
    a[anr] = b[bnr];
    a[anr + 1] = b[bnr + 1];
    a[anr + 2] = b[bnr + 2];
}

// Adds and scales the values of two vertex positions in a floating array
void tissueNode::vecAdd(MFloatArray& a, int anr, MFloatArray& b, int bnr, float scale)
{
    anr *= 3;
    bnr *= 3;
    a[anr] += b[bnr] * scale;
    a[anr + 1] += b[bnr + 1] * scale;
    a[anr + 2] += b[bnr + 2] * scale;
}

// Find the length squared of specific vertex position
float tissueNode::vecLengthSquared(MFloatArray& a, int anr)
{
    anr *= 3;
    float a0 = a[anr], a1 = a[anr + 1], a2 = a[anr + 2];
    return a0 * a0 + a1 * a1 + a2 * a2;
}

void tissueNode::vecSetCross(MFloatArray& a, int anr, MFloatArray& b, int bnr, MFloatArray& c, int cnr)
{
    anr *= 3; bnr *= 3; cnr *= 3;
    a[anr++] = b[bnr + 1] * c[cnr + 2] - b[bnr + 2] * c[cnr + 1];
    a[anr++] = b[bnr + 2] * c[cnr + 0] - b[bnr + 0] * c[cnr + 2];
    a[anr] = b[bnr + 0] * c[cnr + 1] - b[bnr + 1] * c[cnr + 0];
}

// Find the euclidian distance of two specific vertex positions in a floating point array
float tissueNode::vecDistSquared(MFloatArray& a, int anr, MFloatArray& b, int bnr)
{
    anr *= 3; bnr *= 3;
    float a0 = a[anr] - b[bnr], a1 = a[anr + 1] - b[bnr + 1], a2 = a[anr + 2] - b[bnr + 2];
    return a0 * a0 + a1 * a1 + a2 * a2;
}

// Subtract two point arrays from each other
void tissueNode::subtractPointArray(MPointArray& init_points, MPointArray& input_points, MPointArray& output_points)
{
    MPointArray out;
    for (int x = 0; x < (int)input_points.length(); ++x)
    {
        out.append(input_points[x] - init_points[x]);
    }
    output_points = out;

}