//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <stdexcept>
using namespace Eigen;
using namespace std;

rst::pos_buf_id rst::rasterizer::load_positions(const vector<Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const vector<Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

// Bresenham's line drawing algorithm
// Code taken from a stack overflow answer: https://stackoverflow.com/a/16405254
void rst::rasterizer::draw_line(Vector3f begin, Vector3f end)
{
    auto x1 = begin.x();
    auto y1 = begin.y();
    auto x2 = end.x();
    auto y2 = end.y();

    Vector3f line_color = {255, 255, 255};

    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

    dx=x2-x1;
    dy=y2-y1;
    dx1=fabs(dx);
    dy1=fabs(dy);
    px=2*dy1-dx1;
    py=2*dx1-dy1;

    if(dy1<=dx1)//如果线段斜率的绝对值小于等于1就执行下列代码，这样区分是因为像素点坐标是整数，而如果斜率的绝对值小于1，在对线段进行采样时，每次x坐标加1，y坐标要么是加1，要么是减1，要么不变，简化了计算，而当斜率的绝对值大于1时，斜率的倒数的绝对值就小于1，就可以每次对y坐标加1,也是一样的效果
    {
        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
        }
        Vector3f point = Vector3f(x, y, 1.0f);//转换齐次坐标
        set_pixel(point,line_color);
        for(i=0;x<xe;i++)
        {
            x=x+1;
            if(px<0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y=y+1;
                }
                else
                {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
//            delay(0);
            Vector3f point = Vector3f(x, y, 1.0f);
            set_pixel(point,line_color);
        }
    }
    else//如果线段斜率的绝对值大于1，基本处理与上面相似，只是是以y坐标为基准处理
    {
        if(dy>=0)
        {
            x=x1;
            y=y1;
            ye=y2;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
        }
        Vector3f point = Vector3f(x, y, 1.0f);
        set_pixel(point,line_color);
        for(i=0;y<ye;i++)
        {
            y=y+1;
            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x=x+1;
                }
                else
                {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
//            delay(0);
            Vector3f point = Vector3f(x, y, 1.0f);
            set_pixel(point,line_color);
        }
    }
}

auto to_vec4(const Vector3f& v3, float w = 1.0f)//转换齐次坐标
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

void rst::rasterizer::draw(rst::pos_buf_id pos_buffer, rst::ind_buf_id ind_buffer, rst::Primitive type)
{
    if (type != rst::Primitive::Triangle)
    {
        throw runtime_error("Drawing primitives other than triangle is not implemented yet!");
    }
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];

    float f1 = (100 - 0.1) / 2.0;
    float f2 = (100 + 0.1) / 2.0;

    Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;

        Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };

        for (auto& vec : v) {
            vec /= vec.w();
        }

        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }//视口变换

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        t.setColor(0, 255.0,  0.0,  0.0);
        t.setColor(1, 0.0  ,255.0,  0.0);
        t.setColor(2, 0.0  ,  0.0,255.0);

        rasterize_wireframe(t);
    }
}

void rst::rasterizer::rasterize_wireframe(const Triangle& t)
{
    draw_line(t.c(), t.a());
    draw_line(t.c(), t.b());
    draw_line(t.b(), t.a());
}

void rst::rasterizer::set_model(const Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Matrix4f& p)
{
    projection = p;
}

//绕任意轴旋转部分
void rst::rasterizer::set_rodigues(const Matrix4f& r)
{
    rodigues = r;
}

void rst::rasterizer::clear(rst::Buffers buff)//初始化，设置帧缓冲内所有像素颜色为（0，0，0），深度缓冲的所有像素深度为无限大
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        fill(frame_buf.begin(), frame_buf.end(), Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        fill(depth_buf.begin(), depth_buf.end(), numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-y)*width + x;
}

void rst::rasterizer::set_pixel(const Vector3f& point, const Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    if (point.x() < 0 || point.x() >= width ||
        point.y() < 0 || point.y() >= height) return;
    auto ind = (height-point.y())*width + point.x();
    frame_buf[ind] = color;
}

