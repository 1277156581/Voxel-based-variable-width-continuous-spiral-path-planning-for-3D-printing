#include<iostream>
#include<OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>  
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh\Core\Utils\Property.hh> 
#include <string>
#include <vector>
#include <algorithm>
#include<stdio.h>
#include "clipper.hpp"  
#include"clipper.cpp"
#include<GL/glut.h>
//#include"GA.h"
#define use_int32
#define pi 3.1415926
using namespace std;

#define M 700
#define Die 5000
int  die = Die;
int iol = 1;//1Ϊ�ң�2��
double d1 = 0.56, d2 = 0.4;

using namespace std;
struct group {
	vector<int>id;
	double p;
	double fit;
	double sum_p;
};
vector<vector<double>>dis;//�������
vector<int> groupbest;//ÿ�������Ⱦɫ��
vector<group>groups;//Ⱦɫ������

int generation;
double groupbestp;//���Ž��p
double groupbestfit;//���Ž��fit
int changebest;//�Ƿ������Ž��������Ⱥ
double distancenum;



struct element {//��Ԫ�ṹ�壬����洢��ÿ����Ԫ��ȫ����Ϣ
	int id;
	double x, y, z;
	int b;;
};
struct points {//�洢boolΪ1 �ĵ�Ԫ��Ϣ
	int id;
	double x, y, z;
	int b = 0;//�����жϸõ��Ƿ�����
};
struct path {//·���ṹ�壬ÿ��·������һ��˫��
	int id = -1;
		double x, y, z;
};



vector<element>point_buffer1;//��һ�㻺��
vector<vector<element>>point_buffer2;//�ڶ��㻺��
vector<vector<vector<element>>>point;//ÿ����Ԫ����Ϣ
vector<vector<points>>model;//��Ӧ����С������Ч��Ԫ�������򣬴�->С,һά�������
vector<points>model_buffer;
vector<vector<vector<path>>>paths;//�洢·����һά�������
vector<vector<vector<path>>>lines;
vector<path>paths_buffer1;
vector<vector<path>>paths_buffer2;
vector<vector<vector<path>>>paths_buffer3;
vector<vector<path>>path_w;//������
//int num=0;//����
int x_count, y_count, z_count;//�ֱ��ʾx��y,z�ĵ�Ԫ����
double Bmin_x, Bmin_y, Bmax_x, Bmax_y;

void ReadFile() {
	element e;
	ifstream file1, file2,file3;
	file1.open("VoxelizationData.txt");//��ȡ�����ļ�
	//file3.open("path_w.txt");
	double a1, a2;
	int q1 = 428;//����
	path s;
	//for (int i = 0; i < q1; i++) {
	//	file3 >> a1;
	//	file3 >> a2;
	//	if (a1 != 9999 && a2 != 9999) {
	//		s.x = a1;
	//		s.y = a2;
	//		paths_buffer1.push_back(s);
	//	}
	//	else {
	//		path_w.push_back(paths_buffer1);
	//		paths_buffer1.clear();
	//	}
	//}
	//path_w.push_back(paths_buffer1);
	//paths_buffer1.clear();
	file1 >> x_count;
	file1 >> y_count;
	file1 >> z_count;
	printf("%d %d %d \n", x_count, y_count, z_count);
	for (int i = 0; i < z_count; i++) {//��ÿ����Ԫ��������Ϣ��������
		for (int j = 0; j < y_count; j++) {
			for (int m = 0; m < x_count; m++) {
				for (int s = 0; s < 5; s++) {
					switch (s) {
					case 0:file1 >> e.id; break;
					case 1:file1 >> e.x; break;
					case 2:file1 >> e.y; break;
					case 3:file1 >> e.z; break;
					case 4:file1 >> e.b; break;
					}
				}
				point_buffer1.push_back(e);
			}
			point_buffer2.push_back(point_buffer1);
			point_buffer1.clear();
		}
		point.push_back(point_buffer2);
		point_buffer2.clear();
	}
	//for (int i = 0; i < point.size(); i++) {//��ȡ��Ԫ��x,yӦ������
	//	for (int j = 0; j < point[0].size(); j++) {
	//		for (int m = 0; m < point[0][0].size(); m++) {
	//			if (point[i][j][m].b == 1) {
	//				for (int s = 0; s < 4; s++) {
	//					switch (s) {
	//					case 0: {file2 >> e.id; if (e.id != point[i][j][m].id) printf("�ļ���ȡ����\n"); break; }
	//					case 1:file2 >> point[i][j][m].s11; break;
	//					case 2:file2 >> point[i][j][m].s22; break;
	//					case 3:file2 >> point[i][j][m].s12; break;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

double distance(path a, path b)	//��������
{
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

void Init_Element() {
	points t;
	//����ģ�͵�Ԫ
	for (int m = 0; m < point.size(); m++) {
		for (int i = 0; i < point[m].size(); i++) {
			for (int j = 0; j < point[m][i].size(); j++) {
				if (point[m][i][j].b == 1) {
					t.id = point[m][i][j].id;
					t.x = point[m][i][j].x;
					t.y = point[m][i][j].y;
					t.z = point[m][i][j].z;
					t.b = 0; 
					model_buffer.push_back(t);
				}
			}
		}
		model.push_back(model_buffer);
		model_buffer.clear();
	}
}


void VectorInsert(int i, int j) {//���ɵ�·������
	bool flag = true;
	path s;
	int q = 0;
	while (flag) {
		int t = -1;
		int n=-1;
		flag = false;
		if (iol == 1 && t != 1) {
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//down
					if (model[i][m].id == model[i][j].id - x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//down-right
					if (model[i][m].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//right
					if (model[i][m].id == model[i][j].id + 1 && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//right-up
					if (model[i][m].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//up
					if (model[i][m].id == model[i][j].id + x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				iol = 2;
			}
		}

		if (iol == 2 && t != 1) {
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//up
					if (model[i][m].id == model[i][j].id + x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//up-left
					if (model[i][m].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//left
					if (model[i][m].id == model[i][j].id - 1 && model[i][j].id % x_count != 1 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//left-down
					if (model[i][m].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//down
					if (model[i][m].id == model[i][j].id - x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				iol = 1;
			}
		}

		if (iol == 1 && t != 1) {
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//down
					if (model[i][m].id == model[i][j].id - x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//down-right
					if (model[i][m].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//right
					if (model[i][m].id == model[i][j].id + 1 && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//right-up
					if (model[i][m].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				for (int m = 0; m < model[i].size(); m++) {//up
					if (model[i][m].id == model[i][j].id + x_count && model[i][m].b == 0) {
						n = m;
						t = 1;
						break;
					}
				}
			}
			if (t != 1) {
				iol = 2;
			}
		}


		if (t != 1) {
			iol = 1;
		}
		if (t > -1) {
			//printf("1\n");
			model[i][n].b++;
			model[i][j].b++;
			j = n;
			s.id = model[i][n].id;
			s.x = model[i][n].x;
			s.y = model[i][n].y;
			s.z = model[i][n].z;
			paths_buffer1.push_back(s);
			flag = true;
		}
	}
}

void tool_paths() {
	points s;//����
	path n;
	bool flag;
	Init_Element();
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				//model[i][j].b++;//���Ӹõ�Ԫ�Ķ���
				s = model[i][j];
				n.id = s.id;
				n.x = s.x;
				n.y = s.y;
				n.z = s.z;
				paths_buffer1.push_back(n);
				VectorInsert(i, j);
				paths_buffer2.push_back(paths_buffer1);
				paths_buffer1.clear();
			}

		}
		printf("%.2lf%%\r", i * 100.0 / model.size());
		paths.push_back(paths_buffer2);
		paths_buffer2.clear();
	}
}

void double_path() {
	path p;
	int t = 0;//�����ж������յ�
	paths_buffer1.clear();
	paths_buffer2.clear();
	paths_buffer3.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int n = 0; n < 2; n++) {
				if (n == 1) {
					reverse(paths[i][j].begin(), paths[i][j].end());
				}
				for (int m = 0; m < paths[i][j].size(); m++) {
					if (m == 0) {//���
						if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count) {//��->��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count) {//��->��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + 1) {//��->��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - 1) {//��->��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count + 1) {//����->����
							p.x = paths[i][j][m].x - sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count - 1) {//����->����
							p.x = paths[i][j][m].x + sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id - x_count - 1) {//����->����
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m + 1].id == paths[i][j][m].id + x_count + 1) {//����->����
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
					}
					else if (m == paths[i][j].size() - 1) {//�յ�
						if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count) {//��-��
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count) {//��-��
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - 1) {//��-��
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + 1) {//��-��
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1) {//����-����
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1) {//����-����
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + sqrt(2) * d2 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1) {//����-����
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x - sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1) {//����-����
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
							p.x = paths[i][j][m].x + sqrt(2) * d2 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);

						}
					}
					else {
						if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) { //case 1
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1)) {//case 2
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 3
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 + x_count)) {//case 4
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1) {//case 5
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) {//case 6
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1) {//case 7
							p.x = paths[i][j][m].x + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if (paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1) {//case 8
							p.x = paths[i][j][m].x - (d2 * 0.5) * (sqrt(2) * 0.5);
							p.y = paths[i][j][m].y + (d2 * 0.5) * (sqrt(2) * 0.5);
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 9
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1)) {//case 10
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 11
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 12
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 13
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y - d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 14
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y + d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + x_count)) {//case 15
							p.x = paths[i][j][m].x - d1 * 0.5;
							p.y = paths[i][j][m].y + d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1)) {//case 16
							p.x = paths[i][j][m].x + d1 * 0.5;
							p.y = paths[i][j][m].y - d1;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1)) {//case 17
							p.x = paths[i][j][m].x + d1;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count)) {//case 18
							p.x = paths[i][j][m].x - d1;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id - 1) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 + x_count)) {//case 19
							p.x = paths[i][j][m].x - d1;
							p.y = paths[i][j][m].y - d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + 1)) {//case 20
							p.x = paths[i][j][m].x + d1;
							p.y = paths[i][j][m].y + d1 * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count - 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count - 1)) {//case 21
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y - d1 * sqrt(2) * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id + 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id + 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 22
							p.x = paths[i][j][m].x;
							p.y = paths[i][j][m].y + d1 * sqrt(2) * 0.5;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id + x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 + x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 + x_count && paths[i][j][m].id == paths[i][j][m + 1].id + x_count + 1)) {//case 23
							p.x = paths[i][j][m].x - d1 * sqrt(2) * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
						else if ((paths[i][j][m - 1].id == paths[i][j][m].id - x_count + 1 && paths[i][j][m].id == paths[i][j][m + 1].id - 1 - x_count) ||
							(paths[i][j][m - 1].id == paths[i][j][m].id - 1 - x_count && paths[i][j][m].id == paths[i][j][m + 1].id - x_count + 1)) {//case 24
							p.x = paths[i][j][m].x + d1 * sqrt(2) * 0.5;
							p.y = paths[i][j][m].y;
							p.z = paths[i][j][m].z;
							paths_buffer1.push_back(p);
						}
					}
				}
			}
			//paths_buffer1.push_back(paths_buffer1[0]);//�յ����������
			paths_buffer2.push_back(paths_buffer1);
			paths_buffer1.clear();
		}
		paths_buffer3.push_back(paths_buffer2);
		paths_buffer2.clear();
	}
	lines = paths;
	paths.clear();
	paths = paths_buffer3;
	paths_buffer3.clear();
	paths_buffer1.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int m = 0; m < paths[i][j].size() - 1; m += 2) {
				p.x = (paths[i][j][m].x + paths[i][j][m + 1].x) * 0.5;
				p.y = (paths[i][j][m].y + paths[i][j][m + 1].y) * 0.5;
				p.z = (paths[i][j][m].z + paths[i][j][m + 1].z) * 0.5;
				paths_buffer1.push_back(p);
				paths[i][j].insert(paths[i][j].begin() + m + 1, paths_buffer1.begin(), paths_buffer1.end());
				paths_buffer1.clear();

			}
		}
	}
}
void underfill_optimize() {
	path p;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				model[i][j].b++;
				for (int n = 0; n < 4; n++) {
					if (n == 0) {
						p.x = model[i][j].x - d1 * 0.5;
						p.y = model[i][j].y + d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 1) {
						p.x = model[i][j].x + d1 * 0.5;
						p.y = model[i][j].y + d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 2) {
						p.x = model[i][j].x + d1 * 0.5;
						p.y = model[i][j].y - d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
					else if (n == 3) {
						p.x = model[i][j].x - d1 * 0.5;
						p.y = model[i][j].y - d1 * 0.5;
						p.z = model[i][j].z;
						paths_buffer1.push_back(p);
					}
				}

				paths_buffer1.push_back(paths_buffer1[0]);
				paths[i].push_back(paths_buffer1);
				paths_buffer1.clear();
				p.x = model[i][j].x;
				p.y = model[i][j].y;
				p.z = model[i][j].z;
				paths_buffer1.push_back(p);
				lines[i].push_back(paths_buffer1);
				paths_buffer1.clear();
			}
		}
	}
}

void Continuous_path() {

	for (int i = 0; i < paths.size(); i++) {
		for (int j = 1; j < paths[i].size(); j++) {
			for (int n = 0; n < paths[i][j].size() - 1; n++) {
				for (int m = 0; m < paths[i][0].size() - 1; m++) {
					//����
					if (paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][j][n].y == paths[i][j][n + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x && paths[i][0][m].y == paths[i][0][m + 1].y) {
						//˳��
						if (paths[i][j][n].x == paths[i][0][m].x && paths[i][j][n + 1].x == paths[i][0][m + 1].x && abs(paths[i][j][n].y - paths[i][0][m].y) > d1 - 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) < d1 + 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > d1 - 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < d1 + 0.01) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						////����
						else if (paths[i][j][n].x == paths[i][0][m + 1].x && paths[i][j][n + 1].x == paths[i][0][m].x && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > d1 - 0.01 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < d1 + 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > d1 - 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < d1 + 0.01) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//����
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x == paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x == paths[i][0][m + 1].x) {
						//˳��
						if (paths[i][j][n].y == paths[i][0][m].y && paths[i][j][n + 1].y == paths[i][0][m + 1].y && abs(paths[i][j][n].x - paths[i][0][m].x) > d1 - 0.0l && abs(paths[i][j][n].x - paths[i][0][m].x) < d1 + 0.0l && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > d1 - 0.01 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < d1 + 0.01) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (paths[i][j][n].y == paths[i][0][m + 1].y && paths[i][j][n + 1].y == paths[i][0][m].y && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > d1 - 0.001 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) < d1 + 0.001 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > d1 - 0.001 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) < d1 + 0.001) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//б
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.01 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.01 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.01) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n + 1].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n + 1].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.1 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.1) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);//���ұ�
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
				}
				if (j == 0) break;
			}
		}
	}


	//////����������
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 1; j < paths[i].size(); j++) {
			for (int n = 0; n < paths[i][j].size() - 1; n++) {
				for (int m = 0; m < paths[i][0].size() - 1; m++) {
					//����
					if (paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][j][n].y == paths[i][j][n + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x && paths[i][0][m].y == paths[i][0][m + 1].y) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > d1 - 0.1 && abs(paths[i][j][n].y - paths[i][0][m].y) < d1 + 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > d1 - 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < d1 + 0.1) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						////����
						else if (abs(paths[i][j][n].x - paths[i][0][m + 1].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > d1 - 0.1 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < d1 + 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > d1 - 0.1 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < d1 + 0.1) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//����
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x == paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x == paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].y - paths[i][0][m].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].x - paths[i][0][m].x) > d1 - 0.l && abs(paths[i][j][n].x - paths[i][0][m].x) < d1 + 0.l && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > d1 - 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < d1 + 0.1) {
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < (d1 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > (d1 * 0.5) - 0.4 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > d1 - 0.1 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) < d1 + 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > d1 - 0.1 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) < d1 + 0.1) {
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
					//б
					else if (paths[i][j][n].y != paths[i][j][n + 1].y && paths[i][j][n].x != paths[i][j][n + 1].x && paths[i][0][m].y != paths[i][0][m + 1].y && paths[i][0][m].x != paths[i][0][m + 1].x) {
						//˳��
						if (abs(paths[i][j][n].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.4 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.4) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n + 1].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n + 1].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							reverse(paths[i][j].begin(), paths[i][j].begin() + n + 1);
							reverse(paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
						//����
						else if (abs(paths[i][j][n].x - paths[i][0][m + 1].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].x - paths[i][0][m + 1].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n].y - paths[i][0][m + 1].y) > (sqrt(2) * d2 * 0.5) - 0.4 &&
							abs(paths[i][j][n + 1].x - paths[i][0][m].x) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].x - paths[i][0][m].x) > (sqrt(2) * d2 * 0.5) - 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) < (sqrt(2) * d2 * 0.5) + 0.4 && abs(paths[i][j][n + 1].y - paths[i][0][m].y) > (sqrt(2) * d2 * 0.5) - 0.4) {
							paths[i][0][m + 1].x = (paths[i][0][m].x + paths[i][0][m + 1].x) * 0.5;
							paths[i][0][m + 1].y = (paths[i][0][m].y + paths[i][0][m + 1].y) * 0.5;
							paths[i][j][n].x = (paths[i][j][n].x + paths[i][j][n + 1].x) * 0.5;
							paths[i][j][n].y = (paths[i][j][n].y + paths[i][j][n + 1].y) * 0.5;
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin(), paths[i][j].begin() + n + 1);//���ұ�
							paths[i][0].insert(paths[i][0].begin() + m + 1, paths[i][j].begin() + n + 1, paths[i][j].end());
							paths[i].erase(paths[i].begin() + j);
							j = 0; break;
						}
					}
				}
				if (j == 0) break;
			}
		}
	}
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			paths[i][j].push_back(paths[i][j][0]);
		}
	}
	//ɾ��ֱ�ߵ��м������
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			if (paths[i][j].size() > 2) {
				for (int m = 2; m < paths[i][j].size(); m++) {
					if (((paths[i][j][m].x == paths[i][j][m - 1].x && paths[i][j][m - 1].x == paths[i][j][m - 2].x) || (paths[i][j][m].y == paths[i][j][m - 1].y && paths[i][j][m - 1].y == paths[i][j][m - 2].y))
						) {
						paths[i][j].erase(paths[i][j].begin() + m - 1);
						m = m - 1;
					}
				}
			}

		}
	}
}



void GcodePrint() {
	FILE* fp;
	errno_t err;     //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&fp, "ls2.gcode", "a"); //��return 1 , ��ָ������ļ����ļ�����
	double t1 = 0.03326;//���0.2��˿��0.4,�����ֲ���
	double t2 = 0.04756;//���0.2��˿��0.56
	double t0 = t1 + (t2 - t1) / 2;
	double E = 0;
	double r;//�س�
	int L=5;//ƫ����
	fprintf(fp, ";FLAVOR:Marlin\n");
	fprintf(fp, ";Generated with Cura_SteamEngine 4.10.0\n");
	fprintf(fp, "M140 S50\n");
	fprintf(fp, "M105\n");
	fprintf(fp, "M190 S50\n");
	fprintf(fp, "M104 S210\n");
	fprintf(fp, "M105\n");
	fprintf(fp, "M109 S210\n");
	fprintf(fp, "M82 ;absolute extrusion mode\n");
	fprintf(fp, "M201 X500.00 Y500.00 Z100.00 E5000.00 ;Setup machine max acceleration\n");
	fprintf(fp, "M203 X500.00 Y500.00 Z10.00 E50.00 ;Setup machine max feedrate\n");
	fprintf(fp, "M204 P500.00 R1000.00 T500.00 ;Setup Print/Retract/Travel acceleration\n");
	fprintf(fp, "M205 X8.00 Y8.00 Z0.40 E5.00 ;Setup Jerk\n");
	fprintf(fp, "M220 S100 ;Reset Feedrate\n");
	fprintf(fp, "M221 S100 ;Reset Flowrate\n");

	fprintf(fp, "G28 ;Home\n");

	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G1 F2700 E-5\n");
	fprintf(fp, "M107\n");
	

	//fprintf(fp, ";LAYER:0\n");

	//fprintf(fp, "G0 F2000 Z%.1f\n", 0.1);
	//for (int i = 0; Bmin_x + 2 * i * 0.42 + 0.42 < Bmax_x; i++)  //��1��2���ʵ����һ��߶���0.27
	//{
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x + 2 * i * 0.42, Bmin_y);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x + 2 * i * 0.42, Bmax_y, E += (Bmax_y - Bmin_y) * t1);
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x + 2 * i * 0.42 + 0.42, Bmax_y);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x + 2 * i * 0.42 + 0.42, Bmin_y, E += (Bmax_y - Bmin_y) * t1);
	//}
	//fprintf(fp, ";LAYER:1\n");

	//fprintf(fp, "G0 F2000 Z%.1f\n", 0.3);
	//for (int i = 0; Bmin_y + 2 * i * 0.42 + 0.42 < Bmax_y; i++)  //���ǵڶ��㣬�����Ժ���0.2
	//{
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmin_x, Bmin_y + 2 * i * 0.42);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmax_x, Bmin_y + 2 * i * 0.42, E += (Bmax_y - Bmin_y) * t1);
	//	fprintf(fp, "G0 F2000 X%f Y%f \n", Bmax_x, Bmin_y + 2 * i * 0.42 + 0.42);
	//	fprintf(fp, "G1 F1000 X%f Y%f E%f\n", Bmin_x, Bmin_y + 2 * i * 0.42 + 0.42, E += (Bmax_y - Bmin_y) * t1);
	//}
	
	
	
	//ģ�͵�һ��
	
	fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[0][0][0].x+L, paths[0][0][0].y + L, paths[0][0][0].z + 0.1);//ģ�͵�һ����������0.22
	fprintf(fp, "G1 F4800 E0.00000\n");
	
	fprintf(fp, ";TYPE:WALL-OUTPE\n");
	//for (int j = 0; j < path_w.size(); j++) {
	//	r = E - 2;
	//	fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
	//	fprintf(fp, "G0 X%f Y%f\n", path_w[j][0].x + L, path_w[j][0].y + L);
	//	fprintf(fp, "G1 F4800 E%f\n", E);
	//	for (int m = 1; m < path_w[j].size(); m++) {
	//		fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[j][m].x + L, path_w[j][m].y + L, E += distance(path_w[j][m - 1], path_w[j][m]) * t2);
	//	}
	//}
	fprintf(fp, ";TYPE:FILL\n");
	for (int j = 0; j < paths[0].size(); j++) {
		r = E - 2;
		fprintf(fp, "G1 F4800 E%f\n",r);//�س�һ�����룬������˿
		fprintf(fp, "G0 X%f Y%f\n", paths[0][j][0].x + L, paths[0][j][0].y + L);
		fprintf(fp, "G1 F4800 E%f\n", E);
		for (int m = 1; m < paths[0][j].size(); m++) {
			if (paths[0][j][m - 1].x != paths[0][j][m].x && paths[0][j][m - 1].y != paths[0][j][m].y) {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t1);
			}
			else {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x + L, paths[0][j][m].y + L, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t2);
			}
		}
	}

	for (int i = 1; i < paths.size(); i++) {//�ӵڶ��㿪ʼ
		//fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", path_w[0][0].x + L, path_w[0][0].y + L, paths[i][0][0].z + 0.1);
		fprintf(fp, ";TYPE:WALL-OUTPE\n");
		//for (int j = 0; j < path_w.size(); j++) {
		//	r = E - 2;
		//	fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
		//	fprintf(fp, "G0 X%f Y%f\n", path_w[j][0].x + L, path_w[j][0].y + L);
		//	fprintf(fp, "G1 F4800 E%f\n", E);
		//	for (int m = 1; m < path_w[j].size(); m++) {
		//		fprintf(fp, "G1 F1200 X%f Y%f E%f\n", path_w[j][m].x + L, path_w[j][m].y + L, E += distance(path_w[j][m - 1], path_w[j][m]) * t2);
		//	}
		//}
		fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[i][0][0].x + L, paths[i][0][0].y + L, paths[i][0][0].z + 0.1);
		fprintf(fp, ";TYPE:FILL\n");
		for (int j = 0; j < paths[i].size(); j++) {

			r = E - 2;
			fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
			fprintf(fp, "G0 X%f Y%f\n", paths[i][j][0].x + L, paths[i][j][0].y + L);
			fprintf(fp, "G1 F4800 E%f\n", E);

			for (int m = 1; m < paths[i][j].size(); m++) {
				if (paths[i][j][m - 1].x != paths[i][j][m].x && paths[i][j][m - 1].y != paths[i][j][m].y) {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x + L, paths[i][j][m].y + L, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t1);
				}
				else {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x + L, paths[i][j][m].y + L, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t2);
				}
			}
		}
	}
	fprintf(fp, "M140 S0\n");
	fprintf(fp, "M107\n");
	fprintf(fp, "G91\n");
	fprintf(fp, "G1 E-2 F2700\n");
	fprintf(fp, "G1 E-2 Z0.2 F2400 ;Retract and raise Z\n");
	fprintf(fp, "G1 X5 Y5 F3000 ;Wipe out\n");
	fprintf(fp, "G1 Z10 ;Raise Z more\n");
	fprintf(fp, "G90 ;Absolute positioning\n");

	fprintf(fp, "G1 X0 Y300 ;Present print\n");
	fprintf(fp, "M106 S0 ;Turn-off fan\n");
	fprintf(fp, "M104 S0 ;Turn-off hotend\n");
	fprintf(fp, "M140 S0 ;Turn-off bed\n");

	fprintf(fp, "M84 X Y E ;Disable all steppers but Z\n");

	fprintf(fp, "M82 ;absolute extrusion mode\n");
	fprintf(fp, "M104 S0\n");
	fclose(fp);
}



void MatlabData() {
	int d = 0;
	int num;
	FILE* a1;
	errno_t err;//�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&a1, "Matlab_x.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	FILE* b1;    //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&b1, "Matlab_y.txt", "a"); //��return 1 , ��ָ������ļ����ļ�����
	num = paths[paths.size()-1][0].size();
	for (int i = 0; i < paths[paths.size() - 1].size(); i++) {
		if (paths[paths.size() - 1][i].size() > num)
			num = paths[paths.size() - 1][i].size();
	}

	for (int i = 0; i < paths[paths.size() - 1].size(); i++) {
		for (int j = 0; j < num; j++) {
			if (j < paths[paths.size() - 1][i].size()) {
				fprintf(a1, "%f\t", paths[paths.size() - 1][i][j].x);
				fprintf(b1, "%f\t", paths[paths.size() - 1][i][j].y);
			}
			else {
				fprintf(a1, "%f\t",0);
				fprintf(b1, "%f\t",0);
			}
			

		}
		fprintf(a1, "\n");
		fprintf(b1, "\n");
	}
	fclose(a1);
	fclose(b1);
}


//////////////////////////////////////////////////
void main(int argc, char** argv) {
	printf("���ڶ�ȡ�ļ�........\n");
	ReadFile();
	printf("��ȡ�ɹ���\n");
	printf("�������������Ӧ��.......\n");

	printf("����·����........\n");
;
	//��̿��г��Ż�
	tool_paths();
	printf("·�����ɳɹ���\n");
	double_path();
	underfill_optimize();
	Continuous_path();
	GcodePrint();
	printf("���г�����%d\n",paths[0].size());
	//MatlabData();



	int s=0;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				s++;
			}
		}
	}
	printf("��1���ܹ���%d��·��,�յ�����Ϊ%d\n", paths[0].size(),s);
	/*for (int i = 0; i < model[0].size(); i++) {
		printf("%d %f\n",model[0][i].id, model[0][i].s_max);
	}*/
	//for (int i = 0; i < paths.size(); i++) {
		/*for (int j = 0; j < paths[0].size(); j++) {
			for (int m = 0; m < paths[0][j].size(); m++) {
				printf("%d\n",paths[0][j][m].id);
			}
			printf("*************\n");
		}
	printf("%d\n", paths[0].size());*/
	//}
	//�Ż��յ����
	/*int n=0;
	for (int j = 0; j < model[0].size(); j++) {
		if (model[0][j].b == 0) {
			n++;
		}
	}
	printf("%d\n", n);*/
	double dis12=0;
	for (int i = 1; i < paths[0].size(); i++) {
		dis12+=distance(paths[0][i - 1][paths[0][i-1].size() - 1], paths[0][i][0]);
	}
	printf("%d %f\n", paths[0].size()-1,dis12);
}
