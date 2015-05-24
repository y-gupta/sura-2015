#include <iostream>
#include <fstream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc < 2) {
        cout<<"Not enough arguments"<<endl;
        return 0;
    }

    ofstream outfile;
    outfile.open("UV_patch.txt");

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(argv[1], aiProcess_JoinIdenticalVertices);

    if (!scene) {
        cout<<importer.GetErrorString()<<endl;
    }
    else {
        const aiVector3D Zero3D(1.0f, 0.0f, 0.0f);
        for(unsigned int i = 0; i < scene->mNumMeshes; i++) {
            const aiMesh* mesh = scene->mMeshes[i];

            cout<<"Vertex Mesh Count "<<mesh->mNumVertices<<endl;
            for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
                const aiVector3D* texcoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : &Zero3D;

                outfile<<texcoord->x<<" "<<texcoord->y<<endl;
            }

        }
    }

    outfile.close();

    return 0;
}
