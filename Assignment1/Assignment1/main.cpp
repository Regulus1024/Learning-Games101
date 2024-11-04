#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
using namespace Eigen;
using namespace std;

constexpr double MY_PI = 3.1415926;

Matrix4f get_view_matrix(Vector3f eye_pos)
{
    Matrix4f view = Matrix4f::Identity();

    Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Matrix4f get_model_matrix(float rotation_angle)
{
    Matrix4f model = Matrix4f::Identity();
    double angle = (double)rotation_angle / 180.0 * MY_PI;//ת����ת�Ƕ�
    Matrix4f rotation = Matrix4f::Identity();//��z�����ת����
    rotation << cos(angle), -sin(angle), 0, 0,
                sin(angle), cos(angle), 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
    model = rotation * model;
    
    return model;
}

Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    Matrix4f projection = Matrix4f::Identity();

    float alpha = eye_fov / 180.0 * MY_PI;

    float n = -zNear, f = -zFar;
    float t = tan(alpha / 2) * abs(n), r = t * aspect_ratio;
    float b = -t, l = -r;

	Matrix4f Mperspective;
	Mperspective << n, 0, 0, 0,
		            0, n, 0, 0,
		            0, 0, n + f, -n * f,
		            0 , 0, 1, 0;
    Matrix4f translation;
    translation << 1, 0, 0, -(r + l) / 2,
                   0, 1, 0, -(t + b) / 2,
                   0, 0, 1, -(n + f) / 2,
                   0, 0, 0, 1;
    Matrix4f zoom;
    zoom << 2 / (r - l), 0, 0, 0,
            0, 2 / (t - b), 0, 0,
            0, 0, 2 / (n - f), 0,
            0, 0, 0, 1;

    projection = zoom * translation * Mperspective;

    return projection;
}


Matrix4f get_rotation(Vector3f axis, float angle) {//�����������ԭ�������ת�任����

	float alpha = angle / 180 * MY_PI;
	Matrix3f N = Matrix3f::Identity();

	N << 0, -axis.z(), axis.y(),
		axis.z(), 0, -axis.x(),
		-axis.y(), axis.x(), 0;
	Matrix3f rod = cos(alpha) * Matrix3f::Identity() + (1 - cos(alpha)) * axis * axis.transpose() + sin(alpha) * N;

	Matrix4f model = Matrix4f::Identity();

	model << rod(0, 0), rod(0, 1), rod(0, 2), 0,
		     rod(1, 0), rod(1, 1), rod(1, 2), 0,
		     rod(2, 0), rod(2, 1), rod(2, 2), 0,
		     0, 0, 0, 1;
	return model;
}

int main(int argc, const char** argv)
{
    float angle = 0;//����Ƕ�
    bool command_line = false;//���������п��ر�־��Ĭ��Ϊ��
    string filename = "output.png";//�����ļ�����Ĭ��Ϊoutput.png"

    if (argc >= 3) {
        command_line = true;
        angle = stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);//�趨700*700���صĹ�դ���ӿ�

    Vector3f eye_pos = {0, 0, 5};//�趨���λ��

    vector<Vector3f> pos{ {2, 0, -2}, {0, 2, -2}, {-2, 0, -2} };//�趨������λ��

    vector<Vector3i> ind{{0, 1, 2}};//�趨���������,���ڻ�ͼʱȷ����Ҫ���������㣬�����ʾ������������

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);//������ͼ�εĶ�������

    int key = 0;//��������
    int frame_count = 0;//֡���

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);//��ʼ��֡�������Ȼ��棨������ҵ������ҵֻ�漰һ��ͼ�Σ����漰��ȣ�

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));//���դ������MVP����

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    //��������������ת����
    Eigen::Vector3f raxis(0, 0, 1);
    double rangle = 0, ra;
    bool rflag = false;

    cout << "Please enter the axis and angle:" << std::endl;
    cin >> raxis.x() >> raxis.y() >> raxis.z() >> ra;//�����޵����˹��ת��ͽ�

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        if (rflag)//����
        {
            r.set_rodigues(get_rotation(raxis, rangle));
        }
        else
        {
            r.set_rodigues(get_rotation({0,0,1},0));
        }
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {//����a����ʱ����ת10��
            angle += 10;
        }
        else if (key == 'd') {//����d��˳ʱ����ת10��
            angle -= 10;
        }
        else if (key == 'r')//����r���������������ת
        {
            rflag = true;
            rangle += ra;
        }
    }

    return 0;
}
