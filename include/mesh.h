

// Mesh
// vertex buffer
// index buffer
// material


// Mesh Contains data like
// vertices
// indices
// uv
// normal tanget bitangent
// vertex color
enum EVertexAttribute
{
    VertexPosition  = 0,
    VertexNormal    = 1,
    VertexTangent   = 2,
    VertexBiTangent = 3,
    VertexColor     = 4,
    UV0             = 5,
    UV1             = 6,
    UV2             = 7,
    UV3             = 8,
};

class Mesh 
{
    public:
        

    private:
        unsigned int m_vertex_data_mask;
};