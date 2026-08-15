### api
```cpp
cv::cvtColor(bgr, ref, cv::COLOR_BGR2HSV);
```
### 代码
```cpp
void BGR2HSV(Mat &src,Mat &hsv){
    for(int y = 0;y != src.rows;++y){
        for(int x = 0;x !=src.cols;++x){
            
            auto &a = src.at<Vec3b>(y,x);
            float B = a[0]/255.0f;
            float G = a[1]/255.0f;
            float R = a[2]/255.0f;
            float mx = std::max({B,G,R});
            float mn = std::min({B,G,R});
            float d = mx - mn;
            float v = mx;
            float s;
            //v为0时不能为
            if(v > 1e-5f)
                s = d/v;
            else
                s = 0.0f;
            float h = 0.0f;
            //d为0时候不作为除数，会变为无穷大
            if(d!=0){
                if(v > 1e-5f){            
                    if(v == B){
                        h = saturate_cast<float>((240+60*((R-G)/(d))));
                    }else if(v == G){
                        h = saturate_cast<float>((120+60*((B-R)/(d))));
                    }else if(v == R&&a[1]<a[0]){
                        h = saturate_cast<float>((360+60*((G-B)/(d))));
                    }else if(v == R&&a[1]<=a[0]){
                        h = saturate_cast<float>((60*((G-B)/(d))));
                    }
                    if(h<0){
                        h +=360.0f;
                    }
                }
            }
            hsv.at<Vec3f>(y,x) = Vec3f(h,s,v);
        }
    }
}
```
### 涉及公式
- 亮度V为RGB中最大值的结果。
- 饱和度S为RGB最大值与最小值的差值除以最大值。
- 色相H根据RGB的相对大小计算。
	  具体公式如下：
```cpp
if(v == B){
	h = saturate_cast<float>((60*((R-G)/(s*v))));
}else if(v == G){
	h = saturate_cast<float>((120+60*((B-R)/(s*v))));
}else if(v == R&&a[1]<a[0]){
	h = saturate_cast<float>((360+60*((G-B)/(s*v))));
}else if(v == R&&a[1]<=a[0]){
	h = saturate_cast<float>((60*((G-B)/(s*v))));
}
```
### 注意事项：
