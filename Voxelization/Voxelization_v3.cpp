#include <iostream>  
#include <OpenMesh/Core/IO/MeshIO.hh>  
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>  
#include <OpenMesh/Core/Mesh/Handles.hh>
#include <OpenMesh\Core\Utils\Property.hh> 
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include<stdio.h>
#include "clipper.hpp"  
#include"clipper.cpp"
#include<math.h>
#define use_int32
using namespace ClipperLib;
#define eps 1e-8
#define zero(x) (((x)>0?(x):-(x))<eps)
struct point {

	double x, y;

}p, p1;
struct voxelization {//���ص�ṹ��
	double x, y;
	bool b;
};
struct node {
	int id;
	float x, y, z;
};
struct element {
	int n1, n2, n3, n4, n5, n6, n7, n8;
	int id;
};

using namespace std;

typedef unsigned char boolean;

vector<point>coord;  //����һ���������
vector<point>polypoint; //һ�������������
vector<voxelization>v_coord;//����һ���������������
vector<vector<voxelization>>v_coord1;//�����ά������������
vector<vector<voxelization>>v_point;//���ص�,һά��������ά�˲��������ĵ�����
vector<point>tripoint;  //һ��ı仯�� �߶����� �ɶ�
vector<point>new_tripoint;  //�µ�һ��ı仯�� �߶����� �ɶ�
vector<point>interpoint; //��������
vector<vector<point>>model; //slice  ����ģ�͵ģ��ֲ�����������꣬һά��������ά�˲�������ߵ�����
vector<vector<point>>model2; //slice  ����ģ�͵ģ��ֲ�����������꣬һά��������ά�˲�������ߵ�����

vector<vector<vector<voxelization>>>a_point;//������ά�����ģ�����Abaqus
vector<node>node1;
vector<vector<node>>node2;
vector<vector<vector<node>>>node3;
vector<element>a_element;
vector<element>b_element;


//clipper
vector<Paths>model3;//����Ƭ��model��ת�����͵õ��ģ���ƫ��֮����������
vector<point>shellpath;//������ϵĵ�
vector<vector<point> >shellpaths;//��ĳһ������ϵ�·��
vector<vector<vector<point> > >shell2;     //��ĳһ������ϵ�·������  һά�ò�仯����������ά�ǵĲ���·������ά·������

vector<vector<vector<point> > >shell1;//������������ϵ�·��      һά��������άÿ�㲻ͬ�Ľ���·������ά·���ĵ�����
vector<vector<vector<point>>> shell;//����������ϵ�·��    һά��������άÿ�㲻ͬ�Ľ���·������ά��ͬ����·���ǵĲ�������ά��·���������


const string file_1 = "case1.STL";//��ȡ�ļ�

typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;//��������MyMesh�࣬�����������Զ�������
MyMesh mesh;

//���ڰ�Χ��
double Bmax_x, Bmax_y, Bmax_z, Bmin_x, Bmin_y, Bmin_z, px, py, pz;
int countt = 0;
int y_count = 0;//����y��Ԫ�صĸ���

// ��ȡ�ļ��ĺ���
void readfile(string file) {//�ļ���ȡ����mesh����
	// ���󶥵㷨�� vertex normals
	mesh.request_vertex_normals();
	//��������ڶ��㷨�ߣ��򱨴� 
	if (!mesh.has_vertex_normals())
	{
		cout << "���󣺱�׼�������� �����ߡ�������" << endl;
		return;
	}
	// ����ж��㷢�����ȡ�ļ� 
	OpenMesh::IO::Options opt;
	if (!OpenMesh::IO::read_mesh(mesh, file, opt))
	{
		cout << "�޷���ȡ�ļ�:" << file << endl;
		return;
	}
	else cout << "�ɹ���ȡ�ļ�:" << file << endl;
	cout << endl; // Ϊ��ui��ʾ�ÿ�һЩ
				  //��������ڶ��㷨�ߣ�������
	if (!opt.check(OpenMesh::IO::Options::VertexNormal))
	{
		// ͨ���淨�߼��㶥�㷨��
		mesh.request_face_normals();
		// mesh��������㷨��
		mesh.update_normals();
		// �ͷ��淨��
		mesh.release_face_normals();
	}
}

//ͨ���������ȡģ�͵İ�Χ��
void BoundingBox() {
	MyMesh::Point pt;
	int st = 0;
	for (auto it = mesh.vertices_begin(); it != mesh.vertices_end(); ++it) {
		MyMesh::VertexHandle vh_i = *it;
		pt = mesh.point(vh_i);
		px = pt.data()[0];
		py = pt.data()[1];
		pz = pt.data()[2];
		if (st == 0) {
			Bmax_x = Bmin_x = px;
			Bmax_y = Bmin_y = py;
			Bmax_z = Bmin_z = pz;
			st++;
		}
		else {
			if (px > Bmax_x)Bmax_x = px; else if (px < Bmin_x)Bmin_x = px;
			if (py > Bmax_y)Bmax_y = py; else if (py < Bmin_y)Bmin_y = py;
			if (pz > Bmax_z)Bmax_z = pz; else if (pz < Bmin_z)Bmin_z = pz;
		}
	}
}

//���溯��
bool IntersectPlane(MyMesh::Point pt, MyMesh::Point pnorm) //pt������һ�㣬pnorm���淨��
{
	const float ERR = 0.001;
	//�������� pt��pnorm��*pilist��pnum[]  ���庯��ԭ�� �� �����㷨.docx
	int starte, ne, ne1, nf;
	MyMesh::Point vt1, vt2;
	//MyMesh::Face f1;
	MyMesh::HalfedgeHandle nhe;
	MyMesh::FaceHandle nhf;
	float d1, d2, sd1, sd2;
	bool* flag, suc;
	float dist, mind = 1.0e+8;

	sd1 = sd2 = -10000;
	int esize = mesh.n_halfedges();
	flag = new bool[esize];

	suc = false;


	for (MyMesh::HalfedgeIter it = mesh.halfedges_begin(); it != mesh.halfedges_end(); ++it) //��������ģ�����еıߣ��н���İ�id��¼��flag��
	{

		MyMesh::HalfedgeHandle hh = *it;
		int id = hh.idx();
		flag[id] = false;

		auto fromVertex = mesh.from_vertex_handle(hh);
		auto toVertex = mesh.to_vertex_handle(hh);
		vt1 = mesh.point(fromVertex);
		vt2 = mesh.point(toVertex);
		//printf("$ %.3f %.3f $\n", vt1.data()[0],vt2.data()[0]);
		d1 = pnorm.data()[0] * (vt1.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt1.data()[1] - pt.data()[1])
			+ pnorm.data()[2] * (vt1.data()[2] - pt.data()[2]);//d1����ľ���
		d2 = pnorm.data()[0] * (vt2.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt2.data()[1] - pt.data()[1])
			+ pnorm.data()[2] * (vt2.data()[2] - pt.data()[2]);

		if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0) || d1 > 0 && d2 == 0 || d1 == 0 && d2 > 0))//�߶������ཻ
		{
			flag[id] = true;

			vt1.data()[0] = vt1.data()[0] - pt.data()[0];
			vt1.data()[1] = vt1.data()[1] - pt.data()[1];
			vt1.data()[2] = vt1.data()[2] - pt.data()[2];       // point date minus point date 
			dist = vt1.data()[0] * vt1.data()[0] + vt1.data()[1] * vt1.data()[1] + vt1.data()[2] * vt1.data()[2];

			if (dist < mind)
			{
				nhe = hh;	//��̱�
				mind = dist;//��̾���
				ne = id;    //������ڱߵı��               //  printf("ne:  %d  \n", ne);
				suc = true;
			}
		}
	}

	if (!suc)
	{
		delete[]flag;
		return false; //û�н��㣬����return false��������������
	}

	starte = ne;//���ѭ����ʼ�ı�

	suc = false;

	nhf = mesh.face_handle(nhe);//��̱�������

	while (!suc)
	{
		//printf("%%%%");	

		auto fromVertex = mesh.from_vertex_handle(nhe);
		auto toVertex = mesh.to_vertex_handle(nhe);

		vt1 = mesh.point(fromVertex);
		vt2 = mesh.point(toVertex);

		d1 = pnorm.data()[0] * (vt1.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt1.data()[1] - pt.data()[1])
			+ pnorm.data()[2] * (vt1.data()[2] - pt.data()[2]);
		d2 = pnorm.data()[0] * (vt2.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt2.data()[1] - pt.data()[1])
			+ pnorm.data()[2] * (vt2.data()[2] - pt.data()[2]);
		//printf("$$$%lf %lf \n", d1, d2);
		if ((sd1 == d1) && (sd2 == d2))
		{
			flag[ne] = false;
		}
		sd1 = d1; sd2 = d2;
		//pilist[num].data()[0] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[0] - vt1.data()[0]) + vt1.data()[0];
		//pilist[num].data()[1] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[1] - vt1.data()[1]) + vt1.data()[1];
		//pilist[num].data()[2] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[2] - vt1.data()[2]) + vt1.data()[2];

		p = { (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2)) * (vt2.data()[0] - vt1.data()[0]) + vt1.data()[0] ,(float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2)) * (vt2.data()[1] - vt1.data()[1]) + vt1.data()[1] };
		coord.push_back(p);


		do {
			for (auto it = mesh.fh_begin(nhf); it != mesh.fh_end(nhf); ++it) //nhf��̱�������,���������ıߣ�ֻ��3��
			{
				MyMesh::HalfedgeHandle halfnow = *it;

				const int ne1 = halfnow.idx();

				if (flag[ne1] == false || ne == ne1) continue;

				MyMesh::VertexHandle fromV = mesh.from_vertex_handle(halfnow);
				MyMesh::VertexHandle toV = mesh.to_vertex_handle(halfnow);

				vt1 = mesh.point(fromV);
				vt2 = mesh.point(toV);

				d1 = pnorm.data()[0] * (vt1.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt1.data()[1] - pt.data()[1])
					+ pnorm.data()[2] * (vt1.data()[2] - pt.data()[2]);
				d2 = pnorm.data()[0] * (vt2.data()[0] - pt.data()[0]) + pnorm.data()[1] * (vt2.data()[1] - pt.data()[1])
					+ pnorm.data()[2] * (vt2.data()[2] - pt.data()[2]);

				p = { (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2)) * (vt2.data()[0] - vt1.data()[0]) + vt1.data()[0] ,(float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2)) * (vt2.data()[1] - vt1.data()[1]) + vt1.data()[1] };
				coord.push_back(p);

				MyMesh::HalfedgeHandle halfnext = mesh.opposite_halfedge_handle(halfnow);//��ȡ������

				nhf = mesh.face_handle(halfnext);//������������ڵ���

				int ne2 = halfnext.idx();

				flag[ne1] = flag[ne2] = false;//ne1,ne2�Ƕ����������ߣ��������һ���Ľ���Ͷ���Ϊfalse

				if (nhf.idx() == -1)
				{
					starte = ne;
					flag[ne] = false;
					break;
				}
				ne = ne2;//�ԶԱߵ��Ǹ���������һ�������ཻ����

				//pilist[num].data()[0] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[0] - vt1.data()[0]) + vt1.data()[0];
				//pilist[num].data()[1] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[1] - vt1.data()[1]) + vt1.data()[1];
				//pilist[num].data()[2] = (float)fabs(d1) / ((float)fabs(d1) + (float)fabs(d2))*(vt2.data()[2] - vt1.data()[2]) + vt1.data()[2];
				//printf("##%lf %lf %lf\n", pilist[num].data()[0], pilist[num].data()[1], pilist[num].data()[2]);

				break;
			}
		} while (ne != starte);

		suc = true;

		for (auto it = mesh.halfedges_begin(); it != mesh.halfedges_end(); ++it) //������û�еڶ�����
		{

			MyMesh::HalfedgeHandle hh = *it;

			int id = hh.idx();

			if (flag[id] == true)
			{
				ne = id;
				starte = ne;
				nhe = hh;
				nhf = mesh.face_handle(nhe);
				if (nhf.idx() == -1)
				{
					flag[ne] = false;
					continue;
				}
				//pilist[num].data()[0] = -10000;
				//pilist[num].data()[1] = -10000;
				//pilist[num].data()[2] = -10000;
				p = { -10000,-10000 };//�������м�ļ������
				coord.push_back(p);

				suc = false;
				break;
			}
		}


	};

	delete[]flag;
	return true;
}

void findIntersect(float z)
{
	for (double i = Bmin_z+z/2; i < Bmax_z; i += z)
	{
		MyMesh::Normal vf(0, 0, 1);//���淨��
		MyMesh::Point pt; //������һ��
		pt.data()[0] = 0; pt.data()[1] = 0; pt.data()[2] = i;
		IntersectPlane(pt, vf);//����һ�������ߣ�����coord

		model.push_back(coord);
		coord.clear();
	}
	countt = model.size();
}

void clipper()
{
	point p;
	Path  way;
	Paths layer;//ÿ��·������
	IntPoint p1;
	ClipperOffset co;
	Paths solution;
	for (int i = 0; i < model.size(); i++)//����model����i��
	{
		//int m = 0, n = 0;

		for (int j = 0; j < model[i].size(); j++)//����model����i���еĵ�j����
		{
			if (model[i][j].x != -10000)
			{
				p1.X = model[i][j].x * 100;
				p1.Y = model[i][j].y * 100;
				way.push_back(p1);

				//model2[i][m][n].X = (model[i][j].x * 100);
				//model2[i][m][n].Y = (model[i][j].y * 100);
				//n++;
			}
			else
			{
				layer.push_back(way);
				way.clear();
				//m++;
				//n = 0;
			}
		}
		layer.push_back(way);
		way.clear();

		model3.push_back(layer);
		layer.clear();
		//cout << B[0][0].Y << endl;
		//model2.push_back(B);
		//B.clear();


//*********************������Ȧ�ĵ㸳��shell1***********************************
		for (int j = 0; j < model3[i].size(); j++)
		{
			for (int k = 0; k < model3[i][j].size(); k++)
			{
				p.x = double(model3[i][j][k].X) / 100;
				p.y = double(model3[i][j][k].Y) / 100;

				shellpath.push_back(p);
			}
			shellpaths.push_back(shellpath);
			shellpath.clear();
		}
		shell1.push_back(shellpaths);
		shellpaths.clear();



		//******************************��ƫ��***********************************************
		printf("%.2lf%%\r", i * 100.0 / model.size());
		for (int j = 0; j < model3[i].size(); j++)//��ʱ��������model2��ĳ����ĵ�j��path��ƫ��
		{
			double area = abs(Area(model3[i][j]) / 10000);
			co.Clear();
			co.AddPath(model3[i][j], jtRound, etClosedPolygon);   //����׼��ƫ�Ƶ�·��
			int times = 1;	//ƫ�ƴ���������Ǵ�ӡ�Ĳ���
			int sh = 40;     //ƫ�ƺ��

			//cout << "--layer ��" << i << "--path :" << j << endl;		//��ʾ�ڼ��㣬�ڼ���·��
			/*if (area > 115)
			{
				times = 2;
				sh = 20;
			}
			else
			{
				if (area <= 115 && area > 16)
				{
					times = 10;
					sh = -10;
				}
				else
				{
					if (area <= 16 && area > 1)
					{
						times = 2;
						sh = 20;
					}
					else
					{
						times = 10;
						sh = 20;
					}
				}
			}*/


			for (int a = 0; a < times; a++)					 //����ƫ��
			{

				co.Execute(solution, sh);							//ÿ��ƫ�Ƶĺ��

				for (int n = 0; n < solution[0].size(); n++)
				{
					p.x = double(solution[0][n].X) / 100;
					p.y = double(solution[0][n].Y) / 100;


					shellpath.push_back(p);
					//cout << "(" << solution[0][n].X << "," << solution[0][n].Y << ")";
				}

				p.x = double(solution[0][0].X) / 100;
				p.y = double(solution[0][0].Y) / 100;

				shellpath.push_back(p);

				co.Clear();
				co.AddPath(solution[0], jtRound, etClosedPolygon);  //׼����һ��ƫ��
				solution.clear();

				shellpaths.push_back(shellpath);
				shellpath.clear();
			}

			shell2.push_back(shellpaths);
			shellpaths.clear();

		}
		//solution.clear();
		//shell.push_back(shell2);
		//shell2.clear();
	}
}

void InitVoxelization(float x,float y,float z) {//������Χ�����ص�
	voxelization v;
	double v_x, v_y, v_z;
	v_x = Bmin_x + x/2;
	v_y = Bmin_y + y/2;
	v_z = Bmin_z + z/2;
	for (v_y; v_y < Bmax_y; v_y += y) {
		v_x = Bmin_x + x / 2;
		for (v_x; v_x < Bmax_x; v_x += x) {
			v.x = v_x;
			v.y = v_y;
			v.b = false;
			v_coord.push_back(v);
		}
		y_count++;
	}
	for (v_z; v_z < Bmax_z; v_z += z) {
		v_point.push_back(v_coord);
	}
	v_coord.clear();
	printf("��Χ�д�С��x=%.1f,y=%.1f,z=%.1f\n", v_point[0].size() / y_count * x, y_count * y, v_point.size() * z);
}

bool InOrOutPolygon(voxelization a) {//�жϵ��Ƿ��ڶ�����ڣ���true����false��ʹ�õ��������߷�
	double x0 = a.x;
	double y0 = a.y;
	int crossings = 0;
	int n = polypoint.size();
	for (int i = 0; i < n; i++)
	{
		// ��������x֮�� ���Ե㴹ֱy������������
		double slope = (polypoint[(i + 1 + n) % n].y - polypoint[i].y) / (polypoint[(i + 1 + n) % n].x - polypoint[i].x);
		boolean cond1 = (polypoint[i].x <= x0) && (x0 < polypoint[(i + 1 + n) % n].x);
		boolean cond2 = (polypoint[(i + 1 + n) % n].x <= x0) && (x0 < polypoint[i].x);
		boolean above = (y0 < slope* (x0 - polypoint[i].x) + polypoint[i].y);
		if ((cond1 || cond2) && above) crossings++;
	}
	if (crossings % 2 != 0) {
		return true;
	}
	else {
		return false;
	}
}

void intersection() {//����ڶ�����ڵ����ص�
	//for (int i = 0; i < v_point.size(); i++) {
	//	polypoint.clear();//���ϲ�Ļ�����ɾ��
	//	polypoint = model[i];//�ѵ�i����߶�������л���
	//	for (int j = 0; j < v_point[i].size(); j++) {
	//		v_point[i][j].b = InOrOutPolygon(v_point[i][j]);//�жϴ����ص��Ƿ��ڶ������
	//	}
	//}
	for (int h = 0; h < shell1.size(); h++) {
		for (int i = 0; i < shell1[h].size(); i++) {
			for (int m = 0; m < shell1[h][i].size(); m++) {
				shellpath.push_back(shell1[h][i][m]);
			}
			/*for (int h1 = 0; h1 < shell2.size(); h1++) {
				for (int i1 = 0; i1 < shell2[h1].size(); i1++) {
					for (int m1 = 0; m1 < shell2[h1][i1].size(); m1++) {
						shellpath.push_back(shell2[h1][i1][m1]);
					}
				}
			}*/
			for (int h1 = 0; h1 < shell2[0][0].size(); h1++) {
				shellpath.push_back(shell2[0][0][h1]);
			}
			shellpaths.push_back(shellpath);
			shellpath.clear();
		}
		shell.push_back(shellpaths);
		shellpaths.clear();
	}
	for (int h = 0; h < shell.size(); h++)
	{
		for (int i = 0; i < shell[0].size(); i++)
		{
				polypoint.clear();//����һ��������������
				polypoint = shell[0][i];//�ѵ�i��������߸�polypoint
				for (int m = 0; m < v_point[h].size(); m++) {
					v_point[h][m].b = InOrOutPolygon(v_point[h][m]);//�жϴ����ص��Ƿ��ڶ������
				}
			
		}
	}


	

}


void DataOutput(float v_z) {//�����������
	int id = 0;
	float z = v_z / 2;//�����ʵ�Ϊ���ظߵ�һ��
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, "VoxelizationData.txt", "a");
	fprintf(fp, "%d %d %d\n", v_point[0].size() / y_count, y_count, v_point.size());//�ֱ��x,y,z�����ظ���
	for (int i = 0; i < v_point.size(); i++) {
		if (i != 0) {
			z += v_z;
		}
		for (int j = 0; j < v_point[0].size(); j++) {
			id++;
			fprintf(fp, "%d %f %f %f %d\n", id, v_point[i][j].x, v_point[i][j].y, z, v_point[i][j].b);//��Ԫ�ţ�x,y,z�����꣬boolֵ
		}
	}
	fclose(fp);
}

void main(int argc, char** argv) {
	double z = 0.2;//���
	double v_x = 0.56;//���ش�С
	double v_y = 0.56;
	double v_z = 0.2;
	//************************************************************************��Ƭ
	printf("slicing...\n");
	readfile(file_1);
	BoundingBox();
	findIntersect(v_z);  //ǰ����������Ƭ������model
	clipper();
	printf("slice complete,layer count: %d\n", countt);
	//************************************************************************���ػ�
	printf("Voxelizating...\n");
	InitVoxelization(v_x,v_y,v_z);//�������ػ���Χ��
	intersection();//���ػ�ģ��
	DataOutput(v_z);
	printf("Voxelization complete!\n");
}