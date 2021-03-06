#include "modelloader.h"
#include "model.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>

ModelLoader::ModelLoader(Model *model)
{
    m_model = model;
}

ModelLoader::~ModelLoader()
{

}
bool ModelLoader::load(QString pathToFile)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(pathToFile.toStdString(), aiProcess_Triangulate|aiProcess_GenUVCoords|aiProcess_GenSmoothNormals| aiProcess_FlipUVs);

    if (!scene)
    {
        qDebug() << "Error loading file: (assimp:) " << importer.GetErrorString();
        return false;
    }
    if (scene->HasMaterials())
    {
        for (unsigned int ii = 0; ii < scene->mNumMaterials; ++ii)
        {
            QSharedPointer<MaterialInfo> mater = processMaterial(scene->mMaterials[ii]);
            m_materials.push_back(mater);
        }
    }
    if (scene->HasMeshes())
    {
        for (unsigned int ii = 0; ii < scene->mNumMeshes; ++ii)
        {
            m_meshes.push_back(processMesh(scene->mMeshes[ii]));
        }
    }
    else
    {
        qDebug() << "Error: No meshes found";
        return false;
    }
    if (scene->mRootNode != NULL)
    {
        Node *rootNode = new Node;
        processNode(scene, scene->mRootNode, 0, *rootNode);
        m_rootNode.reset(rootNode);
    }
    else
    {
        qDebug() << "Error loading model";
        return false;
    }

    return true;
}

QSharedPointer<MaterialInfo> ModelLoader::processMaterial(aiMaterial *material)
{
    QSharedPointer<MaterialInfo> mater(new MaterialInfo);

    aiString mname;
    material->Get(AI_MATKEY_NAME, mname);
    if (mname.length > 0)
        mater->Name = mname.C_Str();

    int shadingModel;
    material->Get(AI_MATKEY_SHADING_MODEL, shadingModel);

    if (shadingModel != aiShadingMode_Phong && shadingModel != aiShadingMode_Gouraud)
    {
        qDebug() << "This mesh's shading model is not implemented in this loader, setting to default material";
        mater->Name = "DefaultMaterial";
    }
    else
    {
        aiColor3D dif(0.f,0.f,0.f);
        aiColor3D amb(0.f,0.f,0.f);
        aiColor3D spec(0.f,0.f,0.f);
        float shine = 0.0;

        material->Get(AI_MATKEY_COLOR_AMBIENT, amb);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, dif);
        material->Get(AI_MATKEY_COLOR_SPECULAR, spec);
        material->Get(AI_MATKEY_SHININESS, shine);

        mater->Ambient = QVector3D(amb.r, amb.g, amb.b);
        mater->Diffuse = QVector3D(dif.r, dif.g, dif.b);
        mater->Specular = QVector3D(spec.r, spec.g, spec.b);
        mater->Shininess = shine;

        mater->Ambient *= .2;
        if (mater->Shininess == 0.0)
            mater->Shininess = 30;
    }

    return mater;
}

QSharedPointer<Mesh> ModelLoader::processMesh(aiMesh *mesh)
{
    QSharedPointer<Mesh> newMesh(new Mesh);
    newMesh->name = mesh->mName.length != 0 ? mesh->mName.C_Str() : "";
    newMesh->indexOffset = m_indices.size();
    unsigned int indexCountBefore = m_indices.size();
    int vertindexoffset = m_vertices.size()/3;

    // Get Vertices
    if (mesh->mNumVertices > 0)
    {
        for (uint ii = 0; ii < mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mVertices[ii];

            m_vertices.push_back(vec.x);
            m_vertices.push_back(vec.y);
            m_vertices.push_back(vec.z);
        }
    }

    // Get Normals
    if (mesh->HasNormals())
    {
        for (uint ii = 0; ii < mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mNormals[ii];
            m_normals.push_back(vec.x);
            m_normals.push_back(vec.y);
            m_normals.push_back(vec.z);
        };
    }

    // Get Normals
    if (mesh->HasTextureCoords(0))
    {
        for (uint ii = 0; ii < mesh->mNumVertices; ++ii)
        {
            aiVector3D &vec = mesh->mTextureCoords[0][ii];
            m_uv.push_back(vec.x);
            m_uv.push_back(vec.y);
        };
    }
    // Get mesh indexes
    for (uint t = 0; t < mesh->mNumFaces; ++t)
    {
        aiFace* face = &mesh->mFaces[t];
        if (face->mNumIndices != 3)
        {
            qDebug() << "Warning: Mesh face with not exactly 3 indices, ignoring this primitive.";
            continue;
        }

        m_indices.push_back(face->mIndices[0]+vertindexoffset);
        m_indices.push_back(face->mIndices[1]+vertindexoffset);
        m_indices.push_back(face->mIndices[2]+vertindexoffset);
    }

    newMesh->indexCount = m_indices.size() - indexCountBefore;
    newMesh->material = m_materials.at(mesh->mMaterialIndex);

    return newMesh;
}

void ModelLoader::processNode(const aiScene *scene, aiNode *node, Node *parentNode, Node &newNode)
{
    newNode.name = node->mName.length != 0 ? node->mName.C_Str() : "";

    newNode.transformation = QMatrix4x4(node->mTransformation[0]);

    newNode.meshes.resize(node->mNumMeshes);
    for (uint imesh = 0; imesh < node->mNumMeshes; ++imesh)
    {
        QSharedPointer<Mesh> mesh = m_meshes[node->mMeshes[imesh]];
        newNode.meshes[imesh] = mesh;
    }

    for (uint ich = 0; ich < node->mNumChildren; ++ich)
    {
        newNode.nodes.push_back(Node());
        processNode(scene, node->mChildren[ich], parentNode, newNode.nodes[ich]);
    }
}

void ModelLoader::createModel()
{
    m_model->createBuffer(QOpenGLBuffer::StaticDraw);
    m_model->createVertexArrayObjct();
    QOpenGLBuffer *vbo = m_model->getVBO();
    QOpenGLBuffer *ebo = m_model->getEBO();
    QOpenGLVertexArrayObject *vao = m_model->getVertexArrayObject();
    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());

    vao->bind();
    vbo->bind();
    ebo->bind();

    ebo->allocate(sizeof(unsigned int)*(m_indices.size()));
    unsigned int index;
    for(size_t j=0;j < (size_t)m_indices.size();j++)
    {
        index = m_indices[j];
        ebo->write(j*sizeof(unsigned int),&index, sizeof(unsigned int));

    }

    vbo->allocate(sizeof(float)*(m_vertices.size()*2));
    float *data = (float *)vbo->map(QOpenGLBuffer::WriteOnly);

    for(size_t i=0;i < (size_t)(m_vertices.size()/3);i++)
    {
        data[6*i]   = m_vertices[3*i];
        data[6*i+1] = m_vertices[3*i+1];
        data[6*i+2] = m_vertices[3*i+2];
        data[6*i+3] = m_normals[3*i];
        data[6*i+4] = m_normals[3*i+1];
        data[6*i+5] = m_normals[3*i+2];

    }
    vbo->unmap();

    f.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    f.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

    f.glEnableVertexAttribArray(0);
    f.glEnableVertexAttribArray(1);

    vao->release();
    vbo->release();
    ebo->release();
    m_model->setDrawArrays(m_indices.size());

}


void ModelLoader::createRoundedBox(float r, float scale, int n)
{
    int vidx = 0, iidx = 0;
    int vertexCountPerCorner = (n + 2) * (n + 3) / 2;
    int vertexNum = (n+2)*(n+3)*4;
    int indexNum = (n+1)*(n+1)*24+36+72*(n+1);

    m_model->createBuffer(QOpenGLBuffer::StaticDraw);
    m_model->createVertexArrayObjct();
    QOpenGLBuffer *vbo = m_model->getVBO();
    QOpenGLBuffer *ebo = m_model->getEBO();
    QOpenGLVertexArrayObject *vao = m_model->getVertexArrayObject();
    QOpenGLFunctions f = QOpenGLFunctions(QOpenGLContext::currentContext());

    vao->bind();
    vbo->bind();
    ebo->bind();

    vbo->allocate(sizeof(P3T2N3Vertex) * vertexNum);
    P3T2N3Vertex *vp = (P3T2N3Vertex *)vbo->map(QOpenGLBuffer::WriteOnly);
    ebo->allocate(sizeof(unsigned int) * indexNum);
    unsigned int *ip = (unsigned int *)ebo->map(QOpenGLBuffer::WriteOnly);

    for (int corner = 0; corner < 8; ++corner) {
        QVector3D centre(corner & 1 ? 1.0f : -1.0f,
                corner & 2 ? 1.0f : -1.0f,
                corner & 4 ? 1.0f : -1.0f);
        int winding = (corner & 1) ^ ((corner >> 1) & 1) ^ (corner >> 2);
        int offsX = ((corner ^ 1) - corner) * vertexCountPerCorner;
        int offsY = ((corner ^ 2) - corner) * vertexCountPerCorner;
        int offsZ = ((corner ^ 4) - corner) * vertexCountPerCorner;

        // Face polygons
        if (winding) {
            ip[iidx++] = vidx;
            ip[iidx++] = vidx + offsX;
            ip[iidx++] = vidx + offsY;

            ip[iidx++] = vidx + vertexCountPerCorner - n - 2;
            ip[iidx++] = vidx + vertexCountPerCorner - n - 2 + offsY;
            ip[iidx++] = vidx + vertexCountPerCorner - n - 2 + offsZ;

            ip[iidx++] = vidx + vertexCountPerCorner - 1;
            ip[iidx++] = vidx + vertexCountPerCorner - 1 + offsZ;
            ip[iidx++] = vidx + vertexCountPerCorner - 1 + offsX;
        }

        for (int i = 0; i < n + 2; ++i) {

            // Edge polygons
            if (winding && i < n + 1) {
                ip[iidx++] = vidx + i + 1;
                ip[iidx++] = vidx;
                ip[iidx++] = vidx + offsY + i + 1;
                ip[iidx++] = vidx + offsY;
                ip[iidx++] = vidx + offsY + i + 1;
                ip[iidx++] = vidx;

                ip[iidx++] = vidx + i;
                ip[iidx++] = vidx + 2 * i + 2;
                ip[iidx++] = vidx + i + offsX;
                ip[iidx++] = vidx + 2 * i + offsX + 2;
                ip[iidx++] = vidx + i + offsX;
                ip[iidx++] = vidx + 2 * i + 2;

                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 1 - i;
                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 2 - i;
                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 1 - i + offsZ;
                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 2 - i + offsZ;
                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 1 - i + offsZ;
                ip[iidx++] = (corner + 1) * vertexCountPerCorner - 2 - i;
            }

            for (int j = 0; j <= i; ++j) {
                QVector3D normal = QVector3D(i - j, j, n + 1 - i).normalized();
                QVector3D offset(0.5f - r, 0.5f - r, 0.5f - r);
                QVector3D pos = centre * (offset + r * normal);
                vp[vidx].position = scale * pos;
                vp[vidx].normal = centre * normal;
                //vp[vidx].texCoord = QVector2D(pos.x() + 0.5f, pos.y() + 0.5f);

                // Corner polygons
                if (i < n + 1) {
                    ip[iidx++] = vidx;
                    ip[iidx++] = vidx + i + 2 - winding;
                    ip[iidx++] = vidx + i + 1 + winding;
                }
                if (i < n) {
                    ip[iidx++] = vidx + i + 1 + winding;
                    ip[iidx++] = vidx + i + 2 - winding;
                    ip[iidx++] = vidx + 2 * i + 4;
                }

                ++vidx;
            }
        }

    }

    vbo->unmap();

    f.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    f.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

    f.glEnableVertexAttribArray(0);
    f.glEnableVertexAttribArray(1);

    vao->release();
    vbo->release();
    ebo->release();
    m_model->setDrawArrays(indexNum);
}
