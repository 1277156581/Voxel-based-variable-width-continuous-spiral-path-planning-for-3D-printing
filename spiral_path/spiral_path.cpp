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
#define use_int32
#define pi 3.1415926
using namespace std;

struct element {//��Ԫ�ṹ�壬����洢��ÿ����Ԫ��ȫ����Ϣ
	int id;
	double x, y, z, s11, s22, s12, s_max, direction;
	int b;
};
struct points {//�洢boolΪ1 �ĵ�Ԫ��Ϣ
	int id;
	double x, y, z, s_max, direction;
	int b = 0;//�õ�Ԫ�Ķ���
};
struct path {//·���ṹ�壬ÿ��·������һ��˫��
	int id = 0;
	double x, y, z, direction;
};

vector<element>point_buffer1;//��һ�㻺��
vector<vector<element>>point_buffer2;//�ڶ��㻺��
vector<vector<vector<element>>>point;//ÿ����Ԫ����Ϣ
vector<vector<points>>model;//��Ӧ����С������Ч��Ԫ�������򣬴�->С,һά�������
vector<points>model_buffer;
vector<vector<vector<path>>>paths;//�洢·����һά�������
vector<path>paths_buffer1;
vector<vector<path>>paths_buffer2;

int x_count, y_count, z_count;//�ֱ��ʾx��y,z�ĵ�Ԫ����
double Bmin_x, Bmin_y, Bmax_x, Bmax_y;

void ReadFile() {
	element e;
	ifstream file1, file2;
	file1.open("VoxelizationData.txt");//��ȡ�����ļ�
	file2.open("abaqus.txt");//��ȡӦ���ļ�
	file1 >> x_count;
	file1 >> y_count;
	file1 >> z_count;
	file1 >> Bmin_x;
	file1 >> Bmax_x;
	file1 >> Bmin_y;
	file1 >> Bmax_y;
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
	for (int i = 0; i < point.size(); i++) {//��ȡ��Ԫ��x,yӦ������
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].b == 1) {
					for (int s = 0; s < 4; s++) {
						switch (s) {
						case 0: {file2 >> e.id; if (e.id != point[i][j][m].id) printf("�ļ���ȡ����\n"); break; }
						case 1:file2 >> point[i][j][m].s11; break;
						case 2:file2 >> point[i][j][m].s22; break;
						case 3:file2 >> point[i][j][m].s12; break;
						}
					}
				}
			}
		}
	}
}

double distance(path a, path b)	//��������
{
	return sqrt(pow((a.x - b.x), 2) + pow((a.y - b.y), 2));
}

void FE_Analysis() {//���������Ӧ���Լ������Ӧ������
	double s1, s2, d1, d2;
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[i].size(); j++) {
			for (int m = 0; m < point[i][j].size(); m++) {
				if (point[i][j][m].b == 1) {
					//�����Ӧ����ֵ��abs��
					s1 = ((point[i][j][m].s11 + point[i][j][m].s22) / (double)2) - sqrt(pow(((point[i][j][m].s11 - point[i][j][m].s22) / (double)2), 2) + pow(point[i][j][m].s12, 2));
					s2 = ((point[i][j][m].s11 + point[i][j][m].s22) / (double)2) + sqrt(pow(((point[i][j][m].s11 - point[i][j][m].s22) / (double)2), 2) + pow(point[i][j][m].s12, 2));
					if (fabs(s1) >= fabs(s2)) {
						point[i][j][m].s_max = s1;
					}
					else {
						point[i][j][m].s_max = s2;
					}
					//�����Ӧ������
					d1 = atan(-(2 * point[i][j][m].s12) / (point[i][j][m].s11 - point[i][j][m].s22));
					if (point[i][j][m].s11 >= point[i][j][m].s22) {
						if (d1 <= 0) d1 = d1 + 2 * pi;
						
					}
					else if (point[i][j][m].s11 <= point[i][j][m].s22) {
						d1 = d1 + pi;
					}
					d1 = d1 / (double)2;
					//��С��Ӧ������
					d2= atan(-(2 * point[i][j][m].s12) / (point[i][j][m].s11 - point[i][j][m].s22));
					if (point[i][j][m].s11 >= point[i][j][m].s22) {
						d2 = d2 + pi;
					}
					else if (point[i][j][m].s11 <= point[i][j][m].s11) {
						if (d2 <= 0) d2 = d2 + 2 * pi;
					}
					d2 = d2 / (double)2;
					//�����Ӧ����abs������
					if ((s1 > s2 && fabs(s1) > fabs(s2)) || (s2 > s1 && fabs(s2) > fabs(s1))) point[i][j][m].direction = d1;
					else if ((s1 > s2 && fabs(s1) < fabs(s2)) || (s2 > s1 && fabs(s2) < fabs(s1))) point[i][j][m].direction = d2;
				}
			}
		}
	}
}

void VectorInsert(int i,int m) {//���ɵ�·������
	bool flag = true;
	path s;
	while (flag) {
		flag = false;
		if (model[i][m].direction >= 0 && model[i][m].direction < pi / (double)8) {
			
			for (int j = 0; j < model[i].size(); j++) {
				if (model[i][j].id == model[i][m].id + 1 && model[i][m].id % x_count != 0 && model[i][j].b == 0) {//�ұߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					flag = true;
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi / (double)8 && model[i][m].direction < pi * ((double)3 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if (model[i][j].id == model[i][m].id + x_count + 1 && model[i][m].id % x_count != 0 && model[i][j].b == 0) {//���ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					flag = true;
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)3 / (double)8) && model[i][m].direction < pi * ((double)5 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if (model[i][j].id == model[i][m].id + x_count && model[i][j].b == 0) {//�ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					flag = true;
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)5 / (double)8) && model[i][m].direction < pi * ((double)7 / (double)8)) {
			for (int j = 0; j < model[i].size(); j++) {
				if (model[i][j].id == model[i][m].id + x_count - 1 && model[i][m].id % x_count != 1 && model[i][j].b == 0) {//���ϱߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					flag = true;
					break;
				}
			}
		}
		else if (model[i][m].direction >= pi * ((double)7 / (double)8) && model[i][m].direction < pi) {
			for (int j = 0; j < model[i].size(); j++) {
				if (model[i][j].id == model[i][m].id - 1 && model[i][m].id % x_count != 1 && model[i][j].b == 0) {//��ߵĵ�Ԫ
					model[i][m].b++;
					model[i][j].b++;
					m = j;
					s.id = model[i][j].id;
					s.x = model[i][j].x;
					s.y = model[i][j].y;
					s.z = model[i][j].z;
					s.direction = model[i][j].direction;
					paths_buffer1.push_back(s);
					flag = true;
					break;
				}
			}
		}
	}
}

void OptimizePoints() {//�Ż��յ㺯��
	path s;
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				int Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
				int Left_up = 0;
				int Right_down = 0;
				int Left_down = 0;
				for (int m = 0; m < paths[i].size(); m++) {
					for (int n = 1; n < paths[i][m].size(); n++) {
						if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
							(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
							Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id + x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
							(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id + x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
							Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id + 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 0) ||
							(model[i][j].id + 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 0)) {
							Right_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
						if ((model[i][j].id - 1 == paths[i][m][n - 1].id && model[i][j].id - x_count == paths[i][m][n].id && model[i][j].id % x_count != 1) ||
							(model[i][j].id - 1 == paths[i][m][n].id && model[i][j].id - x_count == paths[i][m][n - 1].id && model[i][j].id % x_count != 1)) {
							Left_down = 1;//��������µĶ˵����ӣ���������е�·������
						}
					}
				}
				for (int m = 0; m < paths[i].size(); m++) {
					//��-�յ�
					if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;//Ѱ�ҵ����ӵ�·�����˳��ÿյ��ѭ��
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//����-�յ�
					else if (paths[i][m][paths[i][m].size() - 1].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][0].id == model[i][j].id + 1 && model[i][j].id % x_count != 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][0].id == model[i][j].id + x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][0].id == model[i][j].id - 1 && model[i][j].id % x_count != 1) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//��-���
					else if (paths[i][m][0].id == model[i][j].id - x_count) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][0].id == model[i][j].id + 1 + x_count && model[i][j].id % x_count != 0 && Right_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][0].id == model[i][j].id - 1 + x_count && model[i][j].id % x_count != 1 && Left_up == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][0].id == model[i][j].id - 1 - x_count && model[i][j].id % x_count != 1 && Left_down == 0) {
						model[i][j].b++;
						s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
						paths[i][m].push_back(s);
						for (int f = paths[i][m].size() - 1; f > 0; f--) {
							s = paths[i][m][f];
							paths[i][m][f] = paths[i][m][f - 1];
							paths[i][m][f - 1] = s;
						}
						j = 0;
						break;
					}//����-���
					else if (paths[i][m][0].id == model[i][j].id + 1 - x_count && model[i][j].id % x_count != 0 && Right_down == 0) {
					model[i][j].b++;
					s.id = model[i][j].id; s.x = model[i][j].x; s.y = model[i][j].y; s.z = model[i][j].z; s.direction = model[i][j].direction;
					paths[i][m].push_back(s);
					for (int f = paths[i][m].size() - 1; f > 0; f--) {
						s = paths[i][m][f];
						paths[i][m][f] = paths[i][m][f - 1];
						paths[i][m][f - 1] = s;
					}
					j = 0;
					break;
					}
				}
				
			}
		}
	}
} 

void PathConnect() {
	paths_buffer1.clear();
	int Right_up, Left_up, Right_down, Left_down;
	///////////////////////���ݷ����Ƚ��кϲ����յ�-��㣩
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int m = 0; m < paths[i].size(); m++) {
				for (int n = 1; n < paths[i][m].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			//��
			if (paths[i][j][paths[i][j].size() - 1].direction >= 0 && paths[i][j][paths[i][j].size() - 1].direction < pi / (double)8 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
				for (int m = 0; m < paths[i].size(); m++) {
					if (m == j) m++;
					if (m == paths[i].size()) break;
					if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1) {
						//��m�в��뵽��j�к󷽲�ɾ����m��
						paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
						paths[i].erase(paths[i].begin() + m);
						m = 0; j = 0;
						break;
					}
				}
			}
			//����
			else if (paths[i][j][paths[i][j].size() - 1].direction >= pi / (double)8 && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)3 / (double)8) && paths[i][j][paths[i][j].size() - 1].id % x_count != 0 && Right_up == 0) {
				for (int m = 0; m < paths[i].size(); m++) {
					if (m == j) m++;
					if (m == paths[i].size()) break;
					if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count) {
						//��m�в��뵽��j�к󷽲�ɾ����m��
						paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
						paths[i].erase(paths[i].begin() + m);
						m = 0; j = 0;
						break;
					}
				}
			}
			//��
			else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)3 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)5 / (double)8)) {
				for (int m = 0; m < paths[i].size(); m++) {
					if (m == j) m++;
					if (m == paths[i].size()) break;
					if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + x_count) {
						//��m�в��뵽��j�к󷽲�ɾ����m��
						paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
						paths[i].erase(paths[i].begin() + m);
						m = 0; j = 0;
						break;
					}
				}
			}
			//����
			else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)5 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi * ((double)7 / (double)8) && paths[i][j][paths[i][j].size() - 1].id % x_count != 1 && Left_up == 0) {
				for (int m = 0; m < paths[i].size(); m++) {
					if (m == j) m++;
					if (m == paths[i].size()) break;
					if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + x_count - 1) {
						//��m�в��뵽��j�к󷽲�ɾ����m��
						paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
						paths[i].erase(paths[i].begin() + m);
						m = 0; j = 0;
						break;
					}
				}
			}
			//��
			else if (paths[i][j][paths[i][j].size() - 1].direction >= pi * ((double)7 / (double)8) && paths[i][j][paths[i][j].size() - 1].direction < pi && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
				for (int m = 0; m < paths[i].size(); m++) {
					if (m == j) m++;
					if (m == paths[i].size()) break;
					if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1) {
						//��m�в��뵽��j�к󷽲�ɾ����m��
						paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
						paths[i].erase(paths[i].begin() + m);
						m = 0; j = 0;
						break;
					}
				}
			}
		}
	}
	///////////��������·���ĺϲ�
	//���-���
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//��
				if (paths[i][m][0].id == paths[i][j][0].id + 1 &&
					paths[i][j][0].id % x_count != 0) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//�ϡ���
				else if (paths[i][m][0].id == paths[i][j][0].id + x_count ||
					paths[i][m][0].id == paths[i][j][0].id - x_count) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//��
				else if (paths[i][m][0].id == paths[i][j][0].id - 1 &&
					paths[i][j][0].id % x_count != 1) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}
	}
	//�յ�-�յ�
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//��
				if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 &&
					paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
				//�ϡ���
				else if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + x_count ||
					paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - x_count) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
				//��
				else if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 &&
					paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					j = 0; m = 0;
					break;
				}
			}
		}
	}
	//�յ�-���
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//��
				if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 &&
					paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//�ϡ���
				else if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + x_count ||
					paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - x_count) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//��
				else if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 &&
					paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}
	}
	///////б����
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			//���-���
			Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int m = 0; m < paths[i].size(); m++) {
				for (int n = 1; n < paths[i][m].size(); n++) {
					if ((paths[i][j][0].id + 1 == paths[i][m][n - 1].id && paths[i][j][0].id + x_count == paths[i][m][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][m][n].id && paths[i][j][0].id + x_count == paths[i][m][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][m][n - 1].id && paths[i][j][0].id + x_count == paths[i][m][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][m][n].id && paths[i][j][0].id + x_count == paths[i][m][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id + 1 == paths[i][m][n - 1].id && paths[i][j][0].id - x_count == paths[i][m][n].id && paths[i][j][0].id % x_count != 0) ||
						(paths[i][j][0].id + 1 == paths[i][m][n].id && paths[i][j][0].id - x_count == paths[i][m][n - 1].id && paths[i][j][0].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][0].id - 1 == paths[i][m][n - 1].id && paths[i][j][0].id - x_count == paths[i][m][n].id && paths[i][j][0].id % x_count != 1) ||
						(paths[i][j][0].id - 1 == paths[i][m][n].id && paths[i][j][0].id - x_count == paths[i][m][n - 1].id && paths[i][j][0].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//����
				if (paths[i][m][0].id == paths[i][j][0].id + 1 + x_count && Right_up == 0 && paths[i][j][0].id%x_count!=0) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][0].id - 1 + x_count && Left_up == 0 && paths[i][j][0].id % x_count != 1) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][0].id - 1 - x_count && Left_down == 0&& paths[i][j][0].id % x_count != 1) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][0].id + 1 - x_count && Right_down == 0 && paths[i][j][0].id % x_count != 0) {
					reverse(paths[i][j].begin(), paths[i][j].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
			//�յ�-�յ�
			Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int m = 0; m < paths[i].size(); m++) {
				for (int n = 1; n < paths[i][m].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//����
				if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 0 && paths[i][j][paths[i][j].size() - 1].id%x_count!=0) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][paths[i][m].size() - 1].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
					reverse(paths[i][m].begin(), paths[i][m].end());//��ת
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
			//�յ�-���
			Right_up = 0;//�жϸÿյ��б�����Ƿ�����ӣ���������Ϊ0����������Ϊ1
			Left_up = 0;
			Right_down = 0;
			Left_down = 0;
			for (int m = 0; m < paths[i].size(); m++) {
				for (int n = 1; n < paths[i][m].size(); n++) {
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id + x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_up = 1;//��������ϵĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) ||
						(paths[i][j][paths[i][j].size() - 1].id + 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 0)) {
						Right_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
					if ((paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) ||
						(paths[i][j][paths[i][j].size() - 1].id - 1 == paths[i][m][n].id && paths[i][j][paths[i][j].size() - 1].id - x_count == paths[i][m][n - 1].id && paths[i][j][paths[i][j].size() - 1].id % x_count != 1)) {
						Left_down = 1;//��������µĶ˵����ӣ���������е�·������
					}
				}
			}
			for (int m = 0; m < paths[i].size(); m++) {
				if (m == j) m++;
				if (m == paths[i].size()) break;
				//����
				if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 + x_count && Right_up == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 + x_count && Left_up == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id - 1 - x_count && Left_down == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 1) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
				//����
				else if (paths[i][m][0].id == paths[i][j][paths[i][j].size() - 1].id + 1 - x_count && Right_down == 0 && paths[i][j][paths[i][j].size() - 1].id % x_count != 0) {
					paths[i][j].insert(paths[i][j].end(), paths[i][m].begin(), paths[i][m].end());
					paths[i].erase(paths[i].begin() + m);
					m = 0; j = 0;
					break;
				}
			}
		}
	}
}

void Dijkstra() {
	double arcs, s_begin, s_end;
	paths_buffer1.clear();
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size() - 1; j++) {
			 arcs = distance(paths[i][j][paths[i][j].size() - 1], paths[i][j + 1][0]);
			for (int m = j + 1; m < paths[i].size(); m++) {
				s_begin = distance(paths[i][j][paths[i][j].size() - 1], paths[i][m][0]);
				s_end = distance(paths[i][j][paths[i][j].size() - 1], paths[i][m][paths[i][m].size() - 1]);
				if (s_begin < arcs && s_begin < s_end) {
					arcs = s_begin;
					paths_buffer1 = paths[i][m];
					paths[i][m] = paths[i][j + 1];
					paths[i][j + 1] = paths_buffer1;
					paths_buffer1.clear();
				}
				else if (s_end < arcs && s_end < s_begin) {
					arcs = s_end;
					reverse(paths[i][m].begin(), paths[i][m].end());
					paths_buffer1 = paths[i][m];
					paths[i][m] = paths[i][j + 1];
					paths[i][j + 1] = paths_buffer1;
					paths_buffer1.clear();
				}
			}
		}
	}
}

void StressPath() {
	points s;//����
	path n;
	bool flag;
	//������boolΪ1�ĵ�Ԫ����model��
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].b == 1) {
					s.id = point[i][j][m].id;
					s.x = point[i][j][m].x;
					s.y = point[i][j][m].y;
					s.z = point[i][j][m].z;
					s.s_max = point[i][j][m].s_max;
					s.direction = point[i][j][m].direction;
					model_buffer.push_back(s);
				}

			}
		}
		model.push_back(model_buffer);
		model_buffer.clear();
	}
	//����Ӧ����abs���Ĵ�С�����������
	//����ð������
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size() - 1; j++) {
			flag = false;
			for (int m = model[i].size() - 1; m > j; m--) {
				if (abs(model[i][m - 1].s_max) < abs(model[i][m].s_max)) {
					s = model[i][m - 1];
					model[i][m - 1] = model[i][m];
					model[i][m] = s;
					flag = true;
				}
			}
			if (flag == false) break;
		}
	}
	//����Ӧ��·��
	for (int i = 0; i < model.size(); i++) {
		for (int j = 0; j < model[i].size(); j++) {
			if (model[i][j].b == 0) {
				//model[i][j].b++;//���Ӹõ�Ԫ�Ķ���
				s = model[i][j];
				n.id = s.id;
				n.x = s.x;
				n.y = s.y;
				n.z = s.z;
				n.direction = s.direction;
				paths_buffer1.push_back(n);
				VectorInsert(i, j);
				paths_buffer2.push_back(paths_buffer1);
				paths_buffer1.clear();
			}
			
		}
		paths.push_back(paths_buffer2);
		paths_buffer2.clear();
	}
	//ɾ��ֻ��һ����Ԫ��·��
	for (int i = 0; i < paths.size(); i++) {
		for (int j = 0; j < paths[i].size(); j++) {
			if (paths[i][j].size() == 1) {
				paths[i].erase(paths[i].begin() + j);
				j--;//ÿɾ��һ��Ԫ�أ������ĵ�ַ����ǰ��һ����λ������j��Ҫ-1
			}
		}
	}
	//�Ż�·���յ�
	printf("�Ż��յ�......\n");
	OptimizePoints();
	//����·���Ż�
	printf("����·���Ż�......\n");
	PathConnect();
	//��̿��г��Ż�
	Dijkstra();
	//ɾ��ͬһ��ֱ��
}

void GcodePrint() {
	FILE* fp;
	errno_t err;     //�жϴ��ļ����Ƿ���� ���ڷ���1
	err = fopen_s(&fp, "cura2.gcode", "a"); //��return 1 , ��ָ������ļ����ļ�����
	double t1 = 0.03326;//���0.2��˿��0.4
	double t2 = 0.04654;//���0.2��˿��0.56
	double E = 0;
	double r;//�س�
	fprintf(fp, "M190 S50.000000\n");
	fprintf(fp, "M109 S210.000000\n");
	fprintf(fp, "G21\n");
	fprintf(fp, "G90\n");
	fprintf(fp, "M82\n");
	fprintf(fp, "M107\n");
	fprintf(fp, "G28 X0 Y0\n");
	fprintf(fp, "G28 Z0\n");
	fprintf(fp, "G1 Z15.0 F9000\n");
	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G1 F200 E3\n");
	fprintf(fp, "G92 E0\n");
	fprintf(fp, "G1 F9000\n");
	fprintf(fp, "M117 Printing...\n");
	fprintf(fp, "M106 S255");
	fprintf(fp, "G1 F4800 E-6.00000\n");

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
	fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[0][0][0].x, paths[0][0][0].y, paths[0][0][0].z + 0.1);//0.52ģ�͵�һ����������0.22
	fprintf(fp, "G1 F4800 E0.00000\n");
	for (int j = 0; j < paths[0].size(); j++) {
		r = E - 6;
		fprintf(fp, "G1 F4800 E%f\n",r);//�س�һ�����룬������˿
		fprintf(fp, "G0 X%f Y%f\n", paths[0][j][0].x, paths[0][j][0].y);
		fprintf(fp, "G1 F4800 E%f\n", E);
		for (int m = 1; m < paths[0][j].size(); m++) {
			if (paths[0][j][m - 1].x != paths[0][j][m].x && paths[0][j][m - 1].y != paths[0][j][m].y) {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x, paths[0][j][m].y, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t1);
			}
			else {
				fprintf(fp, "G1 F1200 X%f Y%f E%f\n", paths[0][j][m].x, paths[0][j][m].y, E += distance(paths[0][j][m - 1], paths[0][j][m]) * t2);
			}
		}
	}

	for (int i = 1; i < paths.size(); i++) {//�ӵڶ��㿪ʼ
		fprintf(fp, "G0 F9000 X%f Y%f Z%f\n", paths[i][0][0].x, paths[i][0][0].y, paths[i][0][0].z + 0.1);
		for (int j = 0; j < paths[i].size(); j++) {
			r = E - 6;
			fprintf(fp, "G1 F4800 E%f\n", r);//�س�һ�����룬������˿
			fprintf(fp, "G0 X%f Y%f\n", paths[i][j][0].x, paths[i][j][0].y);
			fprintf(fp, "G1 F4800 E%f\n", E);
			for (int m = 1; m < paths[i][j].size(); m++) {
				if (paths[i][j][m - 1].x != paths[i][j][m].x && paths[i][j][m - 1].y != paths[i][j][m].y) {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x, paths[i][j][m].y, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t1);
				}
				else {
					
					fprintf(fp, "G1 F3000 X%f Y%f E%f\n", paths[i][j][m].x, paths[i][j][m].y, E += distance(paths[i][j][m - 1], paths[i][j][m]) * t2);
				}
			}
		}
	}
	fprintf(fp, "M104 S0\n");
	fprintf(fp, "M140 S0\n");
	fprintf(fp, "G91\n");
	fprintf(fp, "G1 E-1 F300\n");
	fprintf(fp, "G1 Z+0.5 E-5 X-20 Y-20 F9000\n");
	fprintf(fp, "G28 X0 Y0\n");
	fprintf(fp, "M84\n");
	fprintf(fp, "G90\n");
	fprintf(fp, "M81\n");
}

void main(int argc, char** argv) {
	printf("���ڶ�ȡ�ļ�........\n");
	ReadFile();
	printf("��ȡ�ɹ���\n");
	printf("�������������Ӧ��.......\n");
	FE_Analysis();
	printf("����·����........\n");
	StressPath();
	printf("·�����ɳɹ���\n");
	GcodePrint();
	int s1, s2, s3, s4, s5;
	s1 = 0; s2 = 0; s3 = 0; s4 = 0; s5 = 0;
	for (int i = 0; i < point.size(); i++) {
		for (int j = 0; j < point[0].size(); j++) {
			for (int m = 0; m < point[0][0].size(); m++) {
				if (point[i][j][m].direction > 0 && point[i][j][m].direction < pi / 8) s1++;
				else if (point[i][j][m].direction >= (double)pi / (double)8 && point[i][j][m].direction < (double)pi * (double)((double)3 / (double)8)) s2++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)3 / (double)8) && point[i][j][m].direction < (double)pi * (double)((double)5 / (double)8)) s3++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)5 / (double)8) && point[i][j][m].direction < (double)pi * (double)((double)7 / (double)8)) s4++;
				else if (point[i][j][m].direction >= (double)pi * (double)((double)7 / (double)8) && point[i][j][m].direction < (double)pi) s5++;
				//printf("%d %f %f %f %d %f %f %f %f %f\n", point[i][j][m].id, point[i][j][m].x, point[i][j][m].y, point[i][j][m].z, point[i][j][m].b, point[i][j][m].s11, point[i][j][m].s22, point[i][j][m].s12, point[i][j][m].s_max, point[i][j][m].direction);
			}
		}
	}
	printf("%d %d %d %d %d\n", s1, s2, s3, s4,s5);
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

}
