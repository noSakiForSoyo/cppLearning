### api

### 代码
```cpp
void HSV2BGR(Mat &hsv,Mat &bgr){
    for(int y = 0;y!=hsv.rows;++y){
        for(int x = 0;x!=hsv.cols;++x){
            Vec3f &a = hsv.at<Vec3f>(y,x);
            Vec3f &b = bgr.at<Vec3f>(y,x);
            float H = a[0];
            float S = a[1];
            float V = a[2];
            int h60f = std::floor(H/60.0f);//取最接近的颜色
            auto hi = h60f % 6;//保证落在0-5之间
            float f = H/60.0f - h60f;//走过的百分比
            float P = V*(1-S); //最小值
            float Q = V*(1-f*S);//减少量
            float T = V*(1-(1-f)*S);//增加量
            if(hi == 0){
                b[0] = P;
                b[1] = T;
                b[2] = V; 
            }else if(hi == 1){
                b[0] = P;
                b[1] = V;
                b[2] = Q; 
            }else if(hi == 2){
                b[0] = T;
                b[1] = V;
                b[2] = P; 
            }else if(hi == 3){
                b[0] = V;
                b[1] = Q;
                b[2] = P; 
            }else if(hi == 4){
                b[0] = V;
                b[1] = P;
                b[2] = T; 
            }else if(hi == 5){
                b[0] = Q;
                b[1] = P;
                b[2] = V; 
            }
        }
    }
}
```
### 相关公式

```cpp
			auto hi = h60f % 6;//保证落在0-5之间
            float f = H/60.0f - h60f;//走过的百分比
            float P = V*(1-S); //最小值
            float Q = V*(1-f*S);//减少量
            float T = V*(1-(1-f)*S);//增加量
            if(hi == 0){
                b[0] = P;
                b[1] = T;
                b[2] = V; 
            }else if(hi == 1){
                b[0] = P;
                b[1] = V;
                b[2] = Q; 
            }else if(hi == 2){
                b[0] = T;
                b[1] = V;
                b[2] = P; 
            }else if(hi == 3){
                b[0] = V;
                b[1] = Q;
                b[2] = P; 
            }else if(hi == 4){
                b[0] = V;
                b[1] = P;
                b[2] = T; 
            }else if(hi == 5){
                b[0] = Q;
                b[1] = P;
                b[2] = V; 
            }
```