#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_rotation(Vector3f a, float angle)
{
    Eigen::Vector3f axis = a.normalized();
    Eigen::Matrix3f r = Eigen::Matrix3f::Zero();
    r = r + std::cos(angle / 180.0 * MY_PI) * Eigen::Matrix3f::Identity();
    Eigen::Matrix3f tensorNN;
    tensorNN << axis(0) * axis(0), axis(0) * axis(1), axis(0) * axis(2),
        axis(1) * axis(0), axis(1) * axis(1), axis(1) * axis(2),
        axis(2) * axis(0), axis(2) * axis(1), axis(2) * axis(2);
    r = r + (1.0 - std::cos(angle / 180.0 * MY_PI)) * tensorNN;
    Eigen::Matrix3f offDiag;
    offDiag << 0, -axis(2), axis(1), axis(2), 0, -axis(0), -axis(1), axis(0), 0;
    r = r + std::sin(angle / 180.0 * MY_PI) * offDiag;

    Eigen::Matrix4f R;
    R << r(0, 0), r(0, 1), r(0, 2), 0, r(1, 0), r(1, 1), r(1, 2), 0, r(2, 0),
        r(2, 1), r(2, 2), 0, 0, 0, 0, 1;
    return R;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float sinT = std::sin(rotation_angle / 180.0 * MY_PI);
    float cosT = std::cos(rotation_angle / 180.0 * MY_PI);
    model(0, 0) = cosT;
    model(0, 1) = -sinT;
    model(1, 0) = sinT;
    model(1, 1) = cosT;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    // Create the projection matrix for the given parameters.
    // Then return it.

    // calculate r l n f t b
    double r, l, n, f, t, b;
    n = zNear;
    f = zFar;
    t = std::abs(n) * std::tan(eye_fov / 2.0 / 180.0 * MY_PI);
    b = -t;
    r = t * aspect_ratio;
    l = -r;
    // first calculate ortho projection
    Eigen::Matrix4f OScale = Eigen::Matrix4f::Identity();
    OScale(0, 0) = 2.0 / (r - l);
    OScale(1, 1) = 2.0 / (t - b);
    OScale(2, 2) = 2.0 / (n - f);
    Eigen::Matrix4f OTranslate = Eigen::Matrix4f::Identity();
    OTranslate(0, 3) = -(r + l) / 2.0;
    OTranslate(1, 3) = -(t + b) / 2.0;
    OTranslate(2, 3) = -(n + f) / 2.0;
    Eigen::Matrix4f O = OScale * OTranslate;
    // then transform from perspective to ortho
    Eigen::Matrix4f PToO = Eigen::Matrix4f::Identity();
    PToO(0, 0) = n;
    PToO(1, 1) = n;
    PToO(2, 2) = n + f;
    PToO(2, 3) = -n * f;
    PToO(3, 2) = 1.0;
    PToO(3, 3) = 0.0;
    // times
    projection = O * PToO;
    // return
    return projection;
}

int main(int argc, const char **argv)
{
    // test diy rotation function
    // Eigen::Vector3f diyAxis(1, 0, 0);
    // float diyTheta = 30;
    // std::cout << " diy rotation matrix is \n"
    //           << get_rotation(diyAxis, diyTheta);
    // exit(0);

    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3)
    {
        command_line = true;
        angle = std::stof(argv[2]);  // -r by default
        if (argc == 4)
        {
            filename = std::string(argv[3]);
        }
        // else
        //     return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        // znear and far must be negetive, otherwise the tri is not in the
        // frustum
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        // znear and far must be negetive, otherwise the tri is not in the
        // frustum
        r.set_projection(get_projection_matrix(45, 1, -0.1, -50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a')
        {
            angle += 10;
        }
        else if (key == 'd')
        {
            angle -= 10;
        }
    }

    return 0;
}
