/*
 * @Author: Divenire
 * @Date: 2021-10-18 10:46:49
 * @LastEditors: Divenire
 * @LastEditTime: 2021-12-02 16:06:51
 * @Description: 
 *              extract ORB cost = 0.0228373 seconds. 
                match ORB cost = 0.000716646 seconds. 
                -- Max dist : 94.000000 
                -- Min dist : 4.000000
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <chrono>

using namespace std;
using namespace cv;

string img1_path = "../data/1.png";
string img2_path = "../data/2.png";


int main(int argc, char **argv) {

  //-- 读取图像
  Mat img_1 = imread(img1_path);
  Mat img_2 = imread(img2_path);
  
  assert(img_1.data != nullptr && img_2.data != nullptr);

  //-- 初始化
  std::vector<KeyPoint> keypoints_1, keypoints_2;
  Mat descriptors_1, descriptors_2;
  Ptr<FeatureDetector> detector_orb = ORB::create(); //ORB检测器
  Ptr<DescriptorExtractor> descriptor = ORB::create(); //ORB描述子

  // 计时开始
  chrono::steady_clock::time_point t1 = chrono::steady_clock::now();

  //-- 第一步:检测 Oriented FAST 角点位置
  detector_orb->detect(img_1, keypoints_1);
  detector_orb->detect(img_2, keypoints_2);

  //-- 第二步:根据角点位置计算 BRIEF 描述子并存储在对应的容器中
  descriptor->compute(img_1, keypoints_1, descriptors_1);
  descriptor->compute(img_2, keypoints_2, descriptors_2);

  //输出提取orb特征所消耗的时间
  chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
  chrono::duration<double> time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
  cout << "extract ORB cost = " << time_used.count() << " seconds. " << endl;
  
  
  

  Mat outimg1;
  drawKeypoints(img_1, keypoints_1, outimg1, Scalar::all(-1), DrawMatchesFlags::DEFAULT);
  imshow("ORB features", outimg1);

  //-- 第三步:对两幅图像中的BRIEF描述子进行匹配，使用 Hamming 距离
  Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming"); //暴力匹配
  vector<DMatch> matches; //保存匹配关系
  t1 = chrono::steady_clock::now();
  matcher->match(descriptors_1, descriptors_2, matches);
  t2 = chrono::steady_clock::now();
  time_used = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
  cout << "match ORB cost = " << time_used.count() << " seconds. " << endl;

  //-- 第四步:匹配点对筛选
  // 计算最小距离和最大距离
  auto min_max = minmax_element(matches.begin(), matches.end(),
                                //lambda函数
                                [](const DMatch &m1, const DMatch &m2) { return m1.distance < m2.distance; });
  double min_dist = min_max.first->distance;
  double max_dist = min_max.second->distance;

  printf("-- Max dist : %f \n", max_dist);
  printf("-- Min dist : %f \n", min_dist);

  //当描述子之间的距离大于两倍的最小距离时,即认为匹配有误.但有时候最小距离会非常小,设置一个经验值30作为下限.
  std::vector<DMatch> good_matches;
  for (int i = 0; i < descriptors_1.rows; i++) {
    if (matches[i].distance <= max(2 * min_dist, 30.0)) {
      good_matches.push_back(matches[i]);
    }
  }

  //-- 第五步:绘制匹配结果
  Mat img_match;
  Mat img_goodmatch;
  drawMatches(img_1, keypoints_1, img_2, keypoints_2, matches, img_match);
  drawMatches(img_1, keypoints_1, img_2, keypoints_2, good_matches, img_goodmatch);
  imshow("all matches", img_match);
  imshow("good matches", img_goodmatch);
  waitKey(0);

  return 0;
}
