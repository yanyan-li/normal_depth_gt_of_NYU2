#include "plane_detection.h"
#include<fstream>

#include<iostream>

using namespace std;
PlaneDetection plane_detection;

void printUsage()
{
	cout << "How to use:  "<<
         "compile:   ./PlaneDetection/build_linux.sh"<<
         "usage:  build/RGBDPlaneDetect RAL-normal RAL-normal/associate.txt RAL-normal/" << endl;
}
void LoadImages(const string &strAssociationFilename, vector<string> &vstrImageFilenamesRGB,
                vector<string> &vstrImageFilenamesD, vector<double> &vTimestamps);
int main(int argc, char** argv)
{
	if (argc != 4 && argc != 5)
	{
		printUsage();
		return -1;
	}

    vector<string> vstrImageFilenamesRGB;
    vector<string> vstrImageFilenamesD;
    vector<double> vTimestamps;
    string strAssociationFilename = string(argv[2]);
    LoadImages(strAssociationFilename, vstrImageFilenamesRGB, vstrImageFilenamesD, vTimestamps);

    int nImages = vstrImageFilenamesRGB.size();
    if(vstrImageFilenamesRGB.empty())
    {
        cerr << endl << "No images found in provided path." << endl;
        return 1;
    }
    else if(vstrImageFilenamesD.size()!=vstrImageFilenamesRGB.size())
    {
        cerr << endl << "Different number of images for rgb and depth." << endl;
        return 1;
    }

    ofstream OutFile("rgb.txt"); //利用构造函数创建txt文本，并且打开该文本
    for(int ni=0; ni<nImages; ni++)
    {
        cout<<"This is the "<<ni<<"th image"<<endl;
        string output_folder = string(argv[3]);
        plane_detection.readDepthImage(string(argv[1])+"/"+vstrImageFilenamesD[ni]);
        plane_detection.readColorImage(string(argv[1])+"/"+vstrImageFilenamesRGB[ni]);
        plane_detection.runPlaneDetection(ni);

        string frame_name =vstrImageFilenamesD[ni];
        frame_name = frame_name.substr(0, frame_name.length() - 10);
        OutFile << vstrImageFilenamesRGB[ni]<<endl;  //把字符串内容"This is a Test!"，写入Test.txt文件
        plane_detection.writeOutputFiles(output_folder, vstrImageFilenamesRGB[ni], ni, false);
    }
    return 0;
}

void LoadImages(const string &strAssociationFilename, vector<string> &vstrImageFilenamesRGB,
                vector<string> &vstrImageFilenamesD, vector<double> &vTimestamps)
{
    ifstream fAssociation;
    fAssociation.open(strAssociationFilename.c_str());
    while(!fAssociation.eof())
    {
        string s;
        getline(fAssociation,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            string sRGB, sD;
            ss >> t;
            vTimestamps.push_back(t);
            ss >> sRGB;
            vstrImageFilenamesRGB.push_back(sRGB);
            ss >> sD;
            vstrImageFilenamesD.push_back(sD);
        }
    }
}
