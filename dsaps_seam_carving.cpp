#include <iostream>
#include <opencv2/opencv.hpp>
#include <math.h>
using namespace cv;
using namespace std;

void MatTo3dArray(Mat &I, uchar ***energies) {
    int channels = I.channels();
    int n_rows = I.rows;
    int n_cols = I.cols * channels;
    
    for (int i = 0; i < n_rows; i ++) {
        uchar *p = I.ptr<uchar>(i);
        for (int j = 0; j < n_cols; j += 3) {
            energies[i][j/3][0] = p[j]; 
            energies[i][j/3][1] = p[j+1];
            energies[i][j/3][2] = p[j+2];
        }
    }
}

float computePixelGradient(uchar ***energies, int curr_i, int curr_j, int right, int left, int down, int up) {
    // computes gradient at pixel(curr_i, curr_j)
    int del_Bx = energies[curr_i][right][0] - energies[curr_i][left][0];
    int del_Gx = energies[curr_i][right][1] - energies[curr_i][left][1];
    int del_Rx = energies[curr_i][right][2] - energies[curr_i][left][2];
    int del_By = energies[down][curr_j][0] - energies[up][curr_j][0];
    int del_Gy = energies[down][curr_j][1] - energies[up][curr_j][1];
    int del_Ry = energies[down][curr_j][2] - energies[up][curr_j][2];
    
    int del_y2 = pow(del_By, 2) + pow(del_Gy, 2) + pow(del_Ry, 2); 
    int del_x2 = pow(del_Bx, 2) + pow(del_Gx, 2) + pow(del_Rx, 2); 
    
    return sqrt(del_y2 + del_x2);
}
void computeGradientArr(uchar ***energies, float **grad_arr, int m, int n) {
    /*inputs: energies->3d-array, grad_arr->2d-array*/
    /*m: number of rows
    **n: number of columns
    */
    for (int i = 0; i < m; i ++) {
        for (int j = 0; j < n; j ++) {
            grad_arr[i][j] = computePixelGradient(energies, i, j, (j + 1) % n, (j - 1 + n) % n, (i + 1) % m, (i - 1 + m) % m);
        }
    }
}

void recomputeGradientVert(uchar ***energies, float **grad_arr, int *vert_seam, int m, int n) {
    for (int i = 0; i < m; i ++) {
        int left = (vert_seam[i] - 1 + n) % n; // col-index of the pixel left to the seam pixel
        grad_arr[i][left] = computePixelGradient(energies, i, left, (left + 1) % n, (left - 1 + n) % n, (i + 1) % m, (i - 1 + m) % m);

        int right = vert_seam[i] % n; // col-index of the pixel right to the seam pixel
        grad_arr[i][right] = computePixelGradient(energies, i, right, (right + 1) % n, (right - 1 + n) % n, (i + 1) % m, (i - 1 + m) % m);
    }

    int down = vert_seam[0]; // col-index of pixel below(wrapped, i.e. @i = 0) the seam pixel
    grad_arr[0][down] = computePixelGradient(energies, 0, down, (down + 1) % n, (down - 1 + n) % n, 1, m-1);
    int up = vert_seam[m-1]; 
    grad_arr[m-1][up] = computePixelGradient(energies, m-1, up, (up + 1) % n, (up - 1 + n) % n, 0, m-2);
}

void recomputeGradientHorz(uchar ***energies, float **grad_arr, int *horz_seam, int m, int n) {
    for (int j = 0; j < n; j ++) {
        int up = (horz_seam[j] - 1 + m) % m; // row-index of the pixel above the seam pixel
        grad_arr[up][j] = computePixelGradient(energies, up, j, (j + 1) % n, (j - 1 + n) % n, (up + 1) % m, (up - 1 + m) % m);

        int down = horz_seam[j] % m; // row-index of the pixel below the seam pixel
        grad_arr[down][j] = computePixelGradient(energies, down, j, (j + 1) % n, (j - 1 + n) % n, (down + 1) % m, (down - 1 + m) % m);
    }

    int right = horz_seam[0]; // row-index of pixel right(wrapped, i.e. @j = 0) of the seam pixel
    grad_arr[right][0] = computePixelGradient(energies, right, 0, 1, n-1, (right + 1) % m, (right - 1 + m) % m);
    int left = horz_seam[n-1]; 
    grad_arr[left][n-1] = computePixelGradient(energies, left, n-1, 0, n-2, (left + 1) % m, (left - 1 + m) % m);
}

void shiftGradientHorz(float **grad_arr, int *vert_seam, int m, int n) {
    for (int i = 0; i < m; i ++) {
        for (int j = vert_seam[m-1-i]; j < n-1; j ++) { // for each row, shift the portion from the seam index to the left by 1
            grad_arr[i][j] = grad_arr[i][j + 1];
            grad_arr[i][j] = grad_arr[i][j + 1];
            grad_arr[i][j] = grad_arr[i][j + 1];
        }
    }
}
void shiftEnergyHorz(uchar ***energies, int *vert_seam, int m, int n) {
    for (int i = 0; i < m; i ++) {
        for (int j = vert_seam[m-1-i]; j < n-1; j ++) { // for each row, shift the portion from the seam index to the left by 1
            energies[i][j][0] = energies[i][j + 1][0];
            energies[i][j][1] = energies[i][j + 1][1];
            energies[i][j][2] = energies[i][j + 1][2];
        }
    }
}
void shiftGradientVert(float **grad_arr, int *horz_seam, int m, int n) {
    for (int j = 0; j < n; j ++) {
        for (int i = horz_seam[n-1-j]; i < m-1; i ++) { // for each row, shift the portion from the seam index to the top by 1
            grad_arr[i][j] = grad_arr[i + 1][j];
            grad_arr[i][j] = grad_arr[i + 1][j];
            grad_arr[i][j] = grad_arr[i + 1][j];
        }
    }
}
void shiftEnergyVert(uchar ***energies, int *horz_seam, int m, int n) {
    for (int j = 0; j < n; j ++) {
        for (int i = horz_seam[n-1-j]; i < m-1; i ++) { // for each row, shift the portion from the seam index to the top by 1
            energies[i][j][0] = energies[i + 1][j][0];
            energies[i][j][1] = energies[i + 1][j][1];
            energies[i][j][2] = energies[i + 1][j][2];
        }
    }
}

void findVertSeam(float **grad_arr, int *x_arr, int m, int n) {
    // grad_arr: gradient array
    float **cost = new float*[m];
    for (int i = 0; i < m; i ++) {
        cost[i] = new float[n];
    }

    for (int j = 0; j < n; j ++) {
        cost[0][j] = grad_arr[0][j];
    }
    
    for (int i = 1; i < m; i ++) {
        for (int j = 0; j < n; j ++) {
            // get the minimum of (atmost 3) values just above
            float min_top;
            if (j == 0) {
                min_top = (n > 1) ? min(cost[i-1][j], cost[i-1][j+1]) : cost[i-1][j];
            }
            else if (j == n-1) { 
                min_top = (n > 1) ? min(cost[i-1][j-1], cost[i-1][j]) : cost[i-1][j];
            }
            else {
                min_top = min(min(cost[i-1][j-1], cost[i-1][j]), cost[i-1][j+1]);
            }
            cost[i][j] = grad_arr[i][j] + min_top;
        }
    }
    
    
    int cnt = 0, j_ = 0;
    float min_cost = LLONG_MAX;
    for (int j = 0; j < n; j ++) {
        if (cost[m-1][j] < min_cost) {
            j_ = j;
            min_cost = cost[m-1][j];
        }
    }
    x_arr[cnt ++] = j_;

    for (int i = m-1; i > 0; i --) {
        if (j_ == 0 && n > 1) {
            j_ = cost[i-1][0] < cost[i-1][1] ? 0 : 1;
        }
        else if (j_ == n-1 && n > 1) {
            j_ = cost[i-1][n-2] < cost[i-1][n-1] ? (n-2) : (n-1);
        }
        else {
            if (cost[i-1][j_-1] <= cost[i-1][j_] && cost[i-1][j_-1] <= cost[i-1][j_+1]) {
                j_ --;
            }
            else if (cost[i-1][j_+1] <= cost[i-1][j_-1] && cost[i-1][j_+1] <= cost[i-1][j_]) {
                j_ ++;
            }
        }
        x_arr[cnt ++] = j_;
    }

    /*deallocation*/
    for (int i = 0; i < m; i ++) {
        delete[] cost[i];
    }
    delete[] cost;   
}

void findHorzSeam(float **grad_arr, int *y_arr, int m, int n) {
    // grad_arr: gradient array
    float **cost = new float*[m];
    for (int i = 0; i < m; i ++) {
        cost[i] = new float[n];
    }

    for (int i = 0; i < m; i ++) {
        cost[i][0] = grad_arr[i][0];
    }
    for (int j = 1; j < n; j ++) {
        for (int i = 0; i < m; i ++) {
            // get the minimum of (atmost 3)values just to the left
            float min_left;
            if (i == 0) {
                min_left = (m > 1) ? min(cost[i][j-1], cost[i+1][j-1]) : cost[i][j-1];
            }
            else if (i == m-1) { 
                min_left = (m > 1) ? min(cost[i][j-1], cost[i-1][j-1]) : cost[i][j-1];
            }
            else {
                min_left = min(min(cost[i][j-1], cost[i-1][j-1]), cost[i+1][j-1]);
            }
            cost[i][j] = grad_arr[i][j] + min_left;
        }
    }
    
    int cnt = 0, i_ = 0;
    float min_cost = LLONG_MAX;
    for (int i = 0; i < m; i ++) {
        if (cost[i][n-1] < min_cost) {
            i_ = i;
            min_cost = cost[i][n-1];
        }
    }
    y_arr[cnt ++] = i_;

    for (int j = n-1; j > 0; j --) {
        if (i_ == 0 && m > 1) {
            i_ = cost[0][j-1] < cost[1][j-1] ? 0 : 1;
        }
        else if (i_ == m-1 && m > 1) {
            i_ = cost[m-2][j-1] < cost[m-1][j-1] ? (m-2) : (m-1);
        }
        else {
            if (cost[i_-1][j-1] <= cost[i_][j-1] && cost[i_-1][j-1] <= cost[i_+1][j-1]) {
                i_ --;
            }
            else if (cost[i_+1][j-1] <= cost[i_][j-1] && cost[i_+1][j-1] <= cost[i_-1][j-1]) {
                i_ ++;
            }
        }
        y_arr[cnt ++] = i_;
    }

    /*deallocation*/
    for (int i = 0; i < m; i ++) {
        delete[] cost[i];
    }
    delete[] cost;
}

void computations(Mat &imp_img) {
    int n_rows = inp_img.rows;
    int n_cols = inp_img.cols;
    cout << "Original Image:\n";
    cout << "Height: " << n_rows << ", Width: " << n_cols << "\n";

    int new_height, new_width;
    cout << "Enter height and width of the new image:\n";
    cin >> new_height >> new_width;
    if (new_height > n_rows || new_width > n_cols) {
        cout << "The dimensions of new image should not exceed the original's\n";
        return 1;
    }

    uchar ***nrg_arr = new uchar**[n_rows];
    for(int i = 0; i < n_rows; i ++) {
        nrg_arr[i] = new uchar*[n_cols];
        for (int j = 0; j < n_cols; j ++) {
            nrg_arr[i][j] = new uchar[3];
        }
    }
    float **grad_arr = new float*[n_rows];
    for(int i = 0; i < n_rows; i ++) {
        grad_arr[i] = new float[n_cols];
    }

    MatTo3dArray(inp_img, nrg_arr);
    computeGradientArr(nrg_arr, grad_arr, n_rows, n_cols);
    
    /*
    uchar gray_scale_arr[n_rows][n_cols];
    for (int i = 0; i < n_rows; i ++) {
        for (int j = 0; j < n_cols; j ++) {
            gray_scale_arr[i][j] = (int)(grad_arr[i][j]/max_grad * 255);
        }
    }
    Mat gray_scale_img(n_rows, n_cols, CV_8U, &gray_scale_arr);
    */

    int del_width = n_cols - new_width;
    int *vert_seam = new int[n_rows]; // list of all the x-values of the vertical seam 
    while (del_width --) {
        findVertSeam(grad_arr, vert_seam, n_rows, n_cols);
        shiftEnergyHorz(nrg_arr, vert_seam, n_rows, n_cols);
        shiftGradientHorz(grad_arr, vert_seam, n_rows, n_cols);
        n_cols --;
        recomputeGradientVert(nrg_arr, grad_arr, vert_seam, n_rows, n_cols);
    }

    int del_height = n_rows - new_height;
    int *horz_seam = new int[n_cols]; // list of all the y-values of the horizontal seam
    while (del_height --) {
        findHorzSeam(grad_arr, horz_seam, n_rows, n_cols);
        shiftEnergyVert(nrg_arr, horz_seam, n_rows, n_cols);
        shiftGradientVert(grad_arr, horz_seam, n_rows, n_cols);
        n_rows --;
        recomputeGradientHorz(nrg_arr, grad_arr, horz_seam, n_rows, n_cols);
    }


    /* Create Mat object that is the processed image*/
    Mat out_img(n_rows, n_cols, CV_8UC3);
    for (int y = 0; y < n_rows; y ++) {
        for (int x = 0; x < n_cols; x ++) {
            out_img.at<Vec3b>(y, x) = Vec3b(nrg_arr[y][x][0], nrg_arr[y][x][1], nrg_arr[y][x][2]);
        }
    }

    namedWindow("Original Image", WINDOW_AUTOSIZE);
    imshow("Original Image", inp_img);
    namedWindow("Processed Image", WINDOW_AUTOSIZE);
    imshow("Processed Image", out_img);
    imwrite("./output/output-image.jpg", out_img);
    waitKey(0);
    destroyAllWindows();


    /*deallocate the memory consumed by the arrays*/
    for(int i = 0; i < inp_img.rows; i ++) {
        for (int j = 0; j < inp_img.cols; j ++) {
            delete[] nrg_arr[i][j];
        }
        delete[] nrg_arr[i];
    }
    delete[] nrg_arr;

    for (int i = 0; i < inp_img.rows; i ++) {
        delete[] grad_arr[i];
    }
    delete[] grad_arr;
    delete[] vert_seam;
    delete[] horz_seam;
}

int main() {
    string input_image_path;
    cout << "Enter the input image path:\n";
    getline(cin, input_image_path);
    Mat input_img = imread(input_image_path, IMREAD_COLOR);
    if (input_img.empty()) {
        cerr << "Error: Could not load the image\n";
        return 1;
    }

    computations(input_img);

    return 0;
}
