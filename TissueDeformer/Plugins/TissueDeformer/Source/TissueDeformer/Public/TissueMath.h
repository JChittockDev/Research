#include "Math/Vector.h"

class TissueMath
{
public:
    void Barycentric(const FVector& point, const FVector& a, const FVector& b, 
                        const FVector& c, float& u, float& v, float& w);
};