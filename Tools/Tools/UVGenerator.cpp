#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>


using namespace std;

float dist2(pair<float, float> p1, pair<float, float> p2) {
    return sqrt(pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2));
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cout<<"Not enough arguements"<<endl;
        return 0;
    }

    ifstream infile;
    infile.open(argv[1]);
    ofstream outfile;
    outfile.open("UV.txt");

    int numVertices, topValues,neighbours;
    infile>>numVertices>>neighbours>>topValues;

    vector< vector<int>  > graph(numVertices, vector<int>(neighbours));

    for(unsigned int i = 0; i < numVertices; i++) {
        int idx;
        infile>>idx;
        for(unsigned int j = 0; j < neighbours; j++) {
            infile>>graph[idx][j];
        }
    }

    vector< vector< pair<float, float> > > uvValues(numVertices, vector< pair<float, float> > (topValues));

    for(unsigned int i = 0; i < numVertices; i++) {
        for(unsigned int j = 0; j < topValues; j++) {
            infile>>uvValues[i][j].first>>uvValues[i][j].second;
        }
    }


    infile.close();


    for(unsigned int i = 0; i < numVertices; i++) {
        vector<int> count(topValues, 0);
        for(unsigned int j = 0; j < neighbours; j++) {
            int u = 0, v = 0, minDist = -1;
            int idx = graph[i][j];
            if (idx != -1) {
                for(unsigned int k = 0; k < topValues; k++) {
                    for(unsigned int l = 0; l < topValues; l++) {
                        float d = dist2(uvValues[i][k], uvValues[idx][l]);
                        if (minDist == -1 || minDist > d) {
                            minDist = d;
                            u = k; v = l;
                        }
                    }
                }
                count[u]++;
            }
        }
        int u = 0;
        for(unsigned int j = 0; j < topValues; j++) {
            if (count[u] < count[j])
                u = j;
        }
        outfile<<uvValues[i][0].first/200.0<<" "<<uvValues[i][0].second/200.0<<endl;
    }

    outfile.close();

    return 0;
}
