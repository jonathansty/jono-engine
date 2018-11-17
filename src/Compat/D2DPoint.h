
struct POINT {
    float x,y;
};

struct RECT {
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};

struct D2D1_POINT_2F {
    float x,y;
};

namespace D2D1{
    class Matrix3x2F {
        public:
        glm::mat3x2 matrix;
    };
}