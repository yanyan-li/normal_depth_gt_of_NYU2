#include "plane_detection.h"
#include <stdint.h>
#include <string>
#include <iomanip>

PlaneDetection::PlaneDetection()
{
	cloud.vertices.resize(kDepthHeight * kDepthWidth);
	cloud.w = kDepthWidth;
	cloud.h = kDepthHeight;
}

PlaneDetection::~PlaneDetection()
{
	cloud.vertices.clear();
	seg_img_.release();
	opt_seg_img_.release();
	color_img_.release();
	opt_membership_img_.release();
	pixel_boundary_flags_.clear();
	pixel_grayval_.clear();
	plane_colors_.clear();
	plane_pixel_nums_.clear();
	opt_plane_pixel_nums_.clear();
	sum_stats_.clear();
	opt_sum_stats_.clear();
}

// Temporarily don't need it since we set intrinsic parameters as constant values in the code.
//bool PlaneDetection::readIntrinsicParameterFile(string filename)
//{
//	ifstream readin(filename, ios::in);
//	if (readin.fail() || readin.eof())
//	{
//		cout << "WARNING: Cannot read intrinsics file " << filename << endl;
//		return false;
//	}
//	string target_str = "m_calibrationDepthIntrinsic";
//	string str_line, str, str_dummy;
//	double dummy;
//	bool read_success = false;
//	while (!readin.eof() && !readin.fail())
//	{
//		getline(readin, str_line);
//		if (readin.eof())
//			break;
//		istringstream iss(str_line);
//		iss >> str;
//		if (str == "m_depthWidth")
//			iss >> str_dummy >> width_;
//		else if (str == "m_depthHeight")
//			iss >> str_dummy >> height_;
//		else if (str == "m_calibrationDepthIntrinsic")
//		{
//			iss >> str_dummy >> fx_ >> dummy >> cx_ >> dummy >> dummy >> fy_ >> cy_;
//			read_success = true;
//			break;
//		}
//	}
//	readin.close();
//	if (read_success)
//	{
//		cloud.vertices.resize(height_ * width_);
//		cloud.w = width_;
//		cloud.h = height_;
//	}
//	return read_success;
//}

bool PlaneDetection::readColorImage(string filename)
{
	color_img_ = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
    //save png image
    //cv::imwrite("/media/repo/dataset/dataset/NYU/training/rgbs/7_"+std::to_string(ni)+".png",color_img_);
	if (color_img_.empty() || color_img_.depth() != CV_8U)
	{
		cout << "ERROR: cannot read color image. No such a file, or the image format is not 8UC3" << endl;
		return false;
	}
	return true;
}

bool PlaneDetection::readDepthImage(string filename)
{
	cv::Mat depth_img = cv::imread(filename, CV_LOAD_IMAGE_UNCHANGED);
	if (depth_img.empty() || depth_img.depth() != CV_8U)
	{
		cout << "WARNING: cannot read depth image. No such a file, or the image format is not 16UC1" << endl;
		//return false;
	}
	int rows = depth_img.rows, cols = depth_img.cols;
	int vertex_idx = 0;
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
            //cout<<(int)depth_img.at<uchar>(i, j)<<endl;
			double z = (double)(depth_img.at<uchar>(i, j)) / kScaleFactor;
            //cout<<"z:"<<z<<endl;
			if (_isnan(z))
			{
				cloud.vertices[vertex_idx++] = VertexType(0, 0, z);
				continue;
			}
			double x = ((double)j - kCx) * z / kFx;
			double y = ((double)i - kCy) * z / kFy;
			cloud.vertices[vertex_idx++] = VertexType(x, y, z);
		}
	}

	return true;
}

bool PlaneDetection::runPlaneDetection(int number)
{
    seg_img_ = cv::Mat(kDepthHeight, kDepthWidth, CV_8UC3);
    cv::Mat mask_img=cv::Mat(kDepthHeight, kDepthWidth, CV_8UC1);
    cv::Mat normal_img=cv::Mat(kDepthHeight, kDepthWidth, CV_8UC3);
    plane_vertices_= std::vector<std::vector<int>>(kDepthHeight,vector<int>(NULL));// (kDepthHeight*kDepthWidth,static_cast<int>(NULL));
    plane_filter.run(&cloud, &plane_vertices_, &seg_img_);

    plane_num_ = (int)plane_vertices_.size();
    // Here we set the plane index of a pixel which does NOT belong to any plane as #planes.

    for (int row = 0; row < kDepthHeight; ++row)
        for (int col = 0; col < kDepthWidth; ++col){
            int numP=plane_filter.membershipImg.at<int>(row, col);
            if(numP>=0)
            {
                mask_img.at<uchar>(row,col)=255;
                normal_img.at<cv::Vec3b>(row,col)[0]=128*(plane_filter.extractedPlanes[numP]->normal[0]+1);
                normal_img.at<cv::Vec3b>(row,col)[1]=128*(plane_filter.extractedPlanes[numP]->normal[1]+1);
                normal_img.at<cv::Vec3b>(row,col)[2]=128*(plane_filter.extractedPlanes[numP]->normal[2]+1);
            }
            else if (plane_filter.membershipImg.at<int>(row, col) < 0){
                mask_img.at<uchar >(row,col)=0;
                plane_filter.membershipImg.at<int>(row, col) = plane_num_;
            }
        }

    // save masks for NYU2
    imwrite("NYU2/mask/"+to_string(number)+".png", mask_img);
    imwrite("NYU2/fittedNormal/"+to_string(number)+".png", normal_img);
    return true;
}

// Note: input filename_prefix is like '/rgbd-image-folder-path/frame-XXXXXX'
void PlaneDetection::writeOutputFiles(string output_folder, string frame_name, int number,bool run_mrf)
{
	computePlaneSumStats(run_mrf);

	if (output_folder.back() != '\\' && output_folder.back() != '/')
		output_folder += "/";	
	string filename_prefix = output_folder + to_string(number);
	cv::imwrite("NYU2/instance/" +to_string(number)+ ".png", seg_img_);
    //writePlaneDataFile(frame_name,number,false);

    cloud.vertices.clear();
    seg_img_.release();
    opt_seg_img_.release();
    color_img_.release();
    opt_membership_img_.release();
    pixel_boundary_flags_.clear();
    pixel_grayval_.clear();
    plane_colors_.clear();
    plane_pixel_nums_.clear();
    opt_plane_pixel_nums_.clear();
    sum_stats_.clear();
    opt_sum_stats_.clear();
	
}

void PlaneDetection::writePlaneDataFile(string filename,int number, bool run_mrf /* = false */)
{
    cv::Mat normal_img=cv::Mat(kDepthHeight, kDepthWidth, CV_8UC3,cv::Scalar(0, 0, 0));

    //vector<cv::Point3f> ManhattanFrame;

    map<int,cv::Point3f> mManhttanPlanes;
	for (int pidx = 0; pidx < plane_num_; ++pidx)
	{
//			out << opt_plane_pixel_nums_[pidx] << " ";

		// Plane color in output image
		int vidx = plane_vertices_[pidx][0];
		cv::Vec3b c = seg_img_.at<cv::Vec3b>(vidx / kDepthWidth, vidx % kDepthWidth);
		//out << int(c.val[2]) << " " << int(c.val[1]) << " "<< int(c.val[0]) << " "; // OpenCV uses BGR by default
        int new_pidx = pid_to_extractedpid[pidx];

        //normal frame
		for(int mm=0;mm<plane_vertices_[pidx].size();mm++)
        {
            int vidxtemp = plane_vertices_[pidx][mm];
            cv::Vec3b c = seg_img_.at<cv::Vec3b>(vidxtemp / kDepthWidth, vidxtemp % kDepthWidth);
            //cout<<c<<", "<<pidx<<", "<<new_pidx<<", "<< vidxtemp<<", pixel?"<<plane_vertices_[pidx].size()<<", "<<vidxtemp / kDepthWidth<<", "<<vidxtemp % kDepthWidth<<endl;
            normal_img.at<cv::Vec3b>(vidxtemp / kDepthWidth,vidxtemp % kDepthWidth)[0]=128*(-plane_filter.extractedPlanes[new_pidx]->normal[0]+1);
            normal_img.at<cv::Vec3b>(vidxtemp / kDepthWidth,vidxtemp % kDepthWidth)[1]=128*(-plane_filter.extractedPlanes[new_pidx]->normal[1]+1);
            normal_img.at<cv::Vec3b>(vidxtemp / kDepthWidth,vidxtemp % kDepthWidth)[2]=128*(-plane_filter.extractedPlanes[new_pidx]->normal[2]+1);
        }
		//save normal vector of the Frame
        cv::Point3f normal;
		normal.x= plane_filter.extractedPlanes[new_pidx]->normal[0];
		normal.y= plane_filter.extractedPlanes[new_pidx]->normal[1];
		normal.z= plane_filter.extractedPlanes[new_pidx]->normal[2];

	}
    cout<<"normal:"<<filename<<endl;
    imwrite("NYU2/a"+filename, normal_img);

    //out.close();
}


void PlaneDetection::computePlaneSumStats(bool run_mrf /* = false */)
{

	sum_stats_.resize(plane_num_);
	for (int pidx = 0; pidx < plane_num_; ++pidx)
	{
		for (int i = 0; i < plane_vertices_[pidx].size(); ++i)
		{
			int vidx = plane_vertices_[pidx][i];
			const VertexType& v = cloud.vertices[vidx];
			sum_stats_[pidx].sx += v[0];		 sum_stats_[pidx].sy += v[1];		  sum_stats_[pidx].sz += v[2];
			sum_stats_[pidx].sxx += v[0] * v[0]; sum_stats_[pidx].syy += v[1] * v[1]; sum_stats_[pidx].szz += v[2] * v[2];
			sum_stats_[pidx].sxy += v[0] * v[1]; sum_stats_[pidx].syz += v[1] * v[2]; sum_stats_[pidx].sxz += v[0] * v[2];
		}
		plane_pixel_nums_.push_back(int(plane_vertices_[pidx].size()));
	}
	for (int pidx = 0; pidx < plane_num_; ++pidx)
	{
		int num = plane_pixel_nums_[pidx];
		sum_stats_[pidx].sx /= num;		sum_stats_[pidx].sy /= num;		sum_stats_[pidx].sz /= num;
		sum_stats_[pidx].sxx /= num;	sum_stats_[pidx].syy /= num;	sum_stats_[pidx].szz /= num;
		sum_stats_[pidx].sxy /= num;	sum_stats_[pidx].syz /= num;	sum_stats_[pidx].sxz /= num;
	}
	// Note that the order of extracted planes in `plane_filter.extractedPlanes` is DIFFERENT from
	// the plane order in `plane_vertices_` after running plane detection function `plane_filter.run()`.
	// So here we compute a mapping between these two types of plane indices by comparing plane centers.
	vector<double> sx(plane_num_), sy(plane_num_), sz(plane_num_);
	for (int i = 0; i < plane_filter.extractedPlanes.size(); ++i)
	{
		sx[i] = plane_filter.extractedPlanes[i]->stats.sx / plane_filter.extractedPlanes[i]->stats.N;
		sy[i] = plane_filter.extractedPlanes[i]->stats.sy / plane_filter.extractedPlanes[i]->stats.N;
		sz[i] = plane_filter.extractedPlanes[i]->stats.sz / plane_filter.extractedPlanes[i]->stats.N;
	}
	extractedpid_to_pid.clear();
	pid_to_extractedpid.clear();
	// If two planes' centers are closest, then the two planes are corresponding to each other.
	for (int i = 0; i < plane_num_; ++i)
	{
		double min_dis = 1000000;
		int min_idx = -1;
		for (int j = 0; j < plane_num_; ++j)
		{
			double a = sum_stats_[i].sx - sx[j], b = sum_stats_[i].sy - sy[j], c = sum_stats_[i].sz - sz[j];
			double dis = a * a + b * b + c * c;
			if (dis < min_dis)
			{
				min_dis = dis;
				min_idx = j;
			}
		}
		if (extractedpid_to_pid.find(min_idx) != extractedpid_to_pid.end())
		{
			cout << "   WARNING: a mapping already exists for extracted plane " << min_idx << ":" << extractedpid_to_pid[min_idx] << " -> " << min_idx << endl;
		}
		pid_to_extractedpid[i] = min_idx;
		extractedpid_to_pid[min_idx] = i;
	}
	if (run_mrf)
	{
		opt_sum_stats_.resize(plane_num_);
		opt_plane_pixel_nums_.resize(plane_num_, 0);
		for (int row = 0; row < kDepthHeight; ++row)
		{
			for (int col = 0; col < kDepthWidth; ++col)
			{
				int label = opt_membership_img_.at<int>(row, col); // plane label each pixel belongs to
				if (label != plane_num_) // pixel belongs to some plane
				{
					opt_plane_pixel_nums_[label]++;
					int vidx = row * kDepthWidth + col;
					const VertexType& v = cloud.vertices[vidx];
					opt_sum_stats_[label].sx += v[0];		  opt_sum_stats_[label].sy += v[1];		    opt_sum_stats_[label].sz += v[2];
					opt_sum_stats_[label].sxx += v[0] * v[0]; opt_sum_stats_[label].syy += v[1] * v[1]; opt_sum_stats_[label].szz += v[2] * v[2];
					opt_sum_stats_[label].sxy += v[0] * v[1]; opt_sum_stats_[label].syz += v[1] * v[2]; opt_sum_stats_[label].sxz += v[0] * v[2];
				}
			}
		}
		for (int pidx = 0; pidx < plane_num_; ++pidx)
		{
			int num = opt_plane_pixel_nums_[pidx];
			opt_sum_stats_[pidx].sx /= num;		opt_sum_stats_[pidx].sy /= num;		opt_sum_stats_[pidx].sz /= num;
			opt_sum_stats_[pidx].sxx /= num;	opt_sum_stats_[pidx].syy /= num;	opt_sum_stats_[pidx].szz /= num;
			opt_sum_stats_[pidx].sxy /= num;	opt_sum_stats_[pidx].syz /= num;	opt_sum_stats_[pidx].sxz /= num;
		}
	}

	//--------------------------------------------------------------
	// Only for debug. It doesn't influence the plane detection.
	for (int pidx = 0; pidx < plane_num_; ++pidx)
	{
		double w = 0;
		//for (int j = 0; j < 3; ++j)
		//	w -= plane_filter.extractedPlanes[pidx]->normal[j] * plane_filter.extractedPlanes[pidx]->center[j];
		w -= plane_filter.extractedPlanes[pidx]->normal[0] * sum_stats_[pidx].sx;
		w -= plane_filter.extractedPlanes[pidx]->normal[1] * sum_stats_[pidx].sy;
		w -= plane_filter.extractedPlanes[pidx]->normal[2] * sum_stats_[pidx].sz;
		double sum = 0;
		for (int i = 0; i < plane_vertices_[pidx].size(); ++i)
		{
			int vidx = plane_vertices_[pidx][i];
			const VertexType& v = cloud.vertices[vidx];
			double dis = w;
			for (int j = 0; j < 3; ++j)
				dis += v[j] * plane_filter.extractedPlanes[pidx]->normal[j];
			sum += dis * dis;
		}
		sum /= plane_vertices_[pidx].size();
		cout << "Distance for plane " << pidx << ": " << sum << endl;
	}
}
