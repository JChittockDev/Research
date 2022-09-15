#include "TissueMath.h"

void TissueMath::Barycentric(const FVector& point, const FVector& a,
    const FVector& b, const FVector& c, float& u, float& v, float& w)
{
    FVector v0 = b - a, v1 = c - a, v2 = point - a;
    float d00 = FVector::DotProduct(v0, v0);
    float d01 = FVector::DotProduct(v0, v1);
    float d11 = FVector::DotProduct(v1, v1);
    float d20 = FVector::DotProduct(v2, v0);
    float d21 = FVector::DotProduct(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
}