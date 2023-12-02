#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>

using namespace std;

class FIR{
        vector<double> _coeffs;
        vector<double> _inputs;
        vector<double> _outputs;
        
        vector<double> _quant_coeffs;
        vector<double> _quant_inputs;
        vector<double> _dequant_outputs;

        vector<int> _fixed_inputs;
        vector<int> _fixed_coeffs;
        vector<int> _fixed_outputs;
        
        double _quant_coeffs_scale,_quant_inputs_scale;

        void _loadfile(string filename, vector<double> &var);
        
        void _quant_fir(int);
        void _truncate();
        void _double2fixed(int wordlength);
    public:
        FIR();
        void reloadfile(string filename, string str);
        void printout(string str);
        void filter_compute();
        void addnoise();
        void fixed_filter_compute(int wordlength);
        void dequant();
        void dumpvec(string filename,string str);
        double snr();
};

FIR::FIR(void){
    this->_loadfile("kaiser_coef.txt",this->_coeffs);
    cout << "Finish to read coeffs" << endl;
    this->_loadfile("input.txt",this->_inputs);
    cout << "Finish to read input signals" << endl;
    this->_quant_coeffs=this->_coeffs;
    this->_quant_inputs=this->_inputs;

}

void FIR::_loadfile(string filename, vector<double> &var){
    ifstream fs;
    string str;
    fs.open(filename);
    if(fs.is_open()){
        while (getline(fs, str)) {
            double num;
            try{
                num = stod(str);
            }catch(invalid_argument& e){
                continue;
            }
            var.push_back(num);            
        }
        fs.close();
    }
    else{
        cout << "No such file!" << endl;
    }
}

void FIR::reloadfile(string filename, string str="coeffs"){
    if(str == "coeffs"){
        this->_coeffs.clear();
        this->_loadfile("kaiser_coef.txt",this->_coeffs);
        cout << "Finish to read coeffs" << endl;
    }
    else if(str == "inputs"){
        this->_inputs.clear();
        this->_loadfile("input.txt",this->_inputs);
        cout << "Finish to read input signals" << endl;
    }
}

void FIR::printout(string str="coeffs"){
    if(str=="coeffs"){
        for(auto &v : this->_coeffs)
            cout << v <<endl;
    }
    else if(str=="inputs"){
        for(auto &v : this->_inputs)
            cout << v <<endl;
    }
    else{
        for(auto &v : this->_outputs)
            cout << v <<endl;
    }

}

void FIR::filter_compute(){
    vector<double> in_buffer(this->_coeffs.size(),0);
    vector<double> coeffs = this->_coeffs;
    double input_x,output_y;
    reverse(coeffs.begin(), coeffs.end());
    this->_outputs.clear();
    for(int i=0;i<this->_coeffs.size()+this->_inputs.size()-1;i++){
    //for(int i=0;i<4;i++){
        if(i<this->_inputs.size())
            input_x = this->_inputs[i];
        else
            input_x = 0;
        output_y = 0;
        in_buffer.erase(in_buffer.begin());
        in_buffer.push_back(input_x);


        for(int j=0;j<coeffs.size();j++){
            output_y += in_buffer[j]*coeffs[j];
        }
        //cout << i << ':' <<output_y << endl;
        this->_outputs.push_back(output_y);
        //break;
    }
    //cout << this->_outputs.size()<<endl;
    
}

void FIR::addnoise(){
    const double mean = 0.0;
    const double stddev = 0.1;
    default_random_engine generator;
    normal_distribution<double> dist(mean, stddev);
    for (auto& v : this->_inputs) {
        v = v + dist(generator)/100;
    }
}

void FIR::_quant_fir(int wl){
    this->_quant_inputs=this->_inputs;
    //this->_addnoise(this->_quant_inputs);
    double max = 0;
    for(auto &v : this->_quant_inputs){
        if(max<abs(v))
            max = abs(v);
    }
    
    this->_quant_inputs_scale = (pow(2,wl-1)-1)/max;
    for(auto &v : this->_quant_inputs)
        v = v * this->_quant_inputs_scale;
    this->_quant_coeffs=this->_coeffs;
    max = 0;
    for(auto &v : this->_quant_coeffs){
        if(max<abs(v))
            max = abs(v);
    }
    this->_quant_coeffs_scale = (pow(2,wl-1)-1)/max;
    for(auto &v : this->_quant_coeffs)
        v = v * this->_quant_coeffs_scale;
}
void FIR::_truncate(){
    /*
    for(int i=0;i<this->_quant_inputs.size();i++){
        this->_fixed_inputs.push_back(this)
    }
    */
    this->_fixed_inputs.clear();
    for(auto &v : this->_quant_inputs){
        this->_fixed_inputs.push_back(int(floor(v)));
        //cout << v <<endl;
        //cout << int(floor(v)) << endl;
    }
    this->_fixed_coeffs.clear();
    for(auto &v : this->_quant_coeffs)
        this->_fixed_coeffs.push_back(int(floor(v)));

}
void FIR::_double2fixed(int wordlength){
    this->_quant_fir(wordlength);
    this->_truncate();

}

void FIR::fixed_filter_compute(int wordlength){
    this->_double2fixed(wordlength);
    vector<int> in_buffer(this->_fixed_coeffs.size(),0);
    vector<int> coeffs = this->_fixed_coeffs;
    int input_x,output_y;
    reverse(coeffs.begin(), coeffs.end());
    this->_fixed_outputs.clear();
    for(int i=0;i<this->_fixed_coeffs.size()+this->_fixed_inputs.size()-1;i++){
        if(i<this->_fixed_inputs.size())
            input_x = this->_fixed_inputs[i];
        else
            input_x = 0;
        output_y = 0;
        in_buffer.erase(in_buffer.begin());
        in_buffer.push_back(input_x);

        for(int j=0;j<coeffs.size();j++){
            output_y += in_buffer[j]*coeffs[j];
        }
        //cout << i << ':' <<output_y << endl;
        this->_fixed_outputs.push_back(output_y);
        //break;
    }
    
    //cout << this->_outputs.size()<<endl;

}

void FIR::dequant(){
    this->_dequant_outputs.clear();
    for(auto &v : this->_fixed_outputs){
        this->_dequant_outputs.push_back(double(v)/this->_quant_coeffs_scale/this->_quant_inputs_scale);
    }
}
void FIR::dumpvec(string filename,string str){
    vector<string> v_str;
    if(str == "outputs"){
        for(auto &v: this->_outputs)
            v_str.push_back(to_string(v));
    }
    else if(str == "dequant_outputs"){
        for(auto &v: this->_dequant_outputs)
            v_str.push_back(to_string(v));
        
            
    }
    else if(str == "fixed_inputs"){
        for(auto &v: this->_fixed_inputs)
            v_str.push_back(to_string(v));
        
    }
    else if(str == "fixed_coeffs"){
        for(auto &v: this->_fixed_coeffs)
            v_str.push_back(to_string(v));
        
    }
    else if(str == "fixed_outputs"){
        for(auto &v: this->_fixed_outputs)
            v_str.push_back(to_string(v));
        
    }
    ofstream ofs;
    ofs.open(filename);
    if (!ofs.is_open()) {
        cout << "Failed to open file."<<endl;
    }
    for (auto &s : v_str) {
        ofs << s <<endl;
    }
}

double FIR::snr(){
    double err,out,s,total_err=0,total_out=0;

    for(int i=0;i<this->_outputs.size();i++){
        out = this->_outputs[i];
        err = this->_outputs[i] - this->_dequant_outputs[i];
        total_err += err*err;
        total_out += out*out;
    }
    s = total_out / total_err;
    s = 20*log10(s);
    return s;
}

int main(){
    vector<int> wl_vec;
    vector<double> snr_vec;
    double snr;
    FIR LPF;
    LPF.filter_compute();
    LPF.addnoise();
    for(int i=0;i<16;i++){
        LPF.fixed_filter_compute(i+1);
        LPF.dequant();
        snr = LPF.snr();
        wl_vec.push_back(i+1);
        snr_vec.push_back(snr);
        //LPF.dumpvec("outputs.txt","outputs");
        //LPF.dumpvec("dequant_outputs.txt","dequant_outputs");
        cout <<"wordlength = " << i+1 << ": snr = " << snr << endl;
    }
    return 0;
}
