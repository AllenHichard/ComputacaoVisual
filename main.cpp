#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <QString>
#include <fstream>
#include <iostream>

/*
 * projeto final do curso de engenharia de computação
 * Componentes: Allen Hichard Marques dos Santos
 *              Allan Pereira
 *              Diogo Lima
 */

using namespace cv;
using namespace std;

Mat segmentacaoThreshold(Mat image);
int exibicao(Mat dst, string titulo);
Mat contorno(Mat src);
Mat preenchimento(Mat src);
Mat interseccaoImagens(Mat img1, Mat img2);
Mat contarCartela(Mat cartela, Mat comprimido);
Mat contar(Mat cartela, Mat comprimido);
int nObjetos;
int nCartela;
Mat converterScala(Mat binaria);
Mat program(Mat atual);
Mat converterRdgGrayScale(Mat image);
Mat TirarSombra(Mat image);
Mat TirarSombra2(Mat image);
Mat thresholdConversorEscalaCinzaSemBackground(Mat image);
Mat segmentacaoThresholdSemComprimido(Mat image2);
Mat cartelaSemComprimido(Mat img1, Mat img2);
Mat sistemaSaida(Mat fundo, Mat comprimido, Mat vazio);
void resultadoTeste();
ofstream f_out;
QString caminho = "entrada-saida/6/";
QString caminhoSalvar = caminho;

void resultadoTeste(){
    string relacao[16];
    string saida[16];
    int i = 0;
    string line;
    QString caminho2 = caminho + "saida.txt";
    ifstream myfile (caminho2.toStdString()); // ifstream = padrão ios:in
    if (myfile.is_open()){
        while (! myfile.eof() ){ //enquanto end of file for false continua
            getline (myfile,line);
            if(line.compare("") != 0){
                saida[i] = line;
                i++;
            }
        }
     myfile.close();
    }
    i = 0;

    caminho2 = caminho + "relacao.txt";
    ifstream myfile2 (caminho2.toStdString()); // ifstream = padrão ios:in
    if (myfile2.is_open()){
        while (! myfile2.eof() ){ //enquanto end of file for false continua
            getline (myfile2,line);
            relacao[i] = line;
            i++;
        }
    myfile.close();
    }
    bool correto = true;
    for(int i = 0; i < 16; i++){
        if(relacao[i].compare(saida[i]) != 0){
            cout << "cartela " << i << " errada" << endl;
            correto = false;
        }
    }
    if(correto){
        cout << "resultado obtido com sucesso - sem falhas" << endl;
    }
}

int main() {

    int tam = 16;
    Mat seqImagem [tam], imagemFinal, rgbcinza;
    QString strArquivos = caminhoSalvar + "%02i.jpg";
     QString caminho2 = caminho + "saida.txt";
    f_out.open(caminho2.toStdString());
    VideoCapture sequencia(strArquivos.toStdString());
    if (!sequencia.isOpened()){
        cerr << "Falha na abertura da sequência de imagens!\n" << endl;
        return 1;
    }
    cout << "Leitura de entreda realizada com sucesso \n" << endl;
    for(int i=0;i<tam;i++){
        sequencia >> seqImagem[i];
    }
    for(int i=0;i<tam;i++){
        //int i = 11;
        nObjetos = 0;
        nCartela = 0;
        cout << "       imagem " << i << endl;
        rgbcinza = thresholdConversorEscalaCinzaSemBackground(seqImagem[i]);
        imagemFinal = program(rgbcinza);
        QString a = caminhoSalvar + "saida " + QString::number(i) + ".jpg";
        exibicao(imagemFinal, a.toStdString());
        //semComprimido = segmentacaoThresholdSemComprimido(rgbcinza.clone());
        //cartelaSemComprimido(soComprimido, semComprimido);
        f_out << endl;
        cout << endl;
    }
    f_out.close();
    resultadoTeste();
    cv::waitKey(0);
    return 1;
}

Mat cartelaSemComprimido(Mat img1, Mat img2){
    CvPoint p;
    p.x = 0;
    p.y = 0;
    cv::Mat imagemCinza = converterScala(img1);
    int height = img1.rows;
    int width = img1.cols;
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int cor = imagemCinza.at<uchar>(i,j);
            if(cor == 255){
                p.x = j;
                p.y = i;
                floodFill(imagemCinza,p,127);
                floodFill(img2,p,0);
            }
        }
    }
    return img2;
}


Mat segmentacaoThresholdSemComprimido(Mat image2){
    image2 = TirarSombra2(image2);
    image2 = converterRdgGrayScale(image2);
    cv::threshold(image2,image2,120,255,cv::THRESH_BINARY);
    Mat elem1 = cv::getStructuringElement( cv::MORPH_RECT, cv::Size(10, 10)); //Elemento estruturante
    cv::imwrite("elemento1.png", elem1);
    cv::morphologyEx(image2,image2,cv::MORPH_ERODE, elem1);;
    cv::blur(image2, image2,Size(30,30));
    cv::threshold(image2,image2,0,255,THRESH_BINARY+THRESH_OTSU);
    cv::morphologyEx(image2,image2,cv::MORPH_DILATE, elem1);
    //exibicao(image2, "Sem comprimidos"); // passo 1
    return image2;
}


Mat thresholdConversorEscalaCinzaSemBackground(Mat image){
    //exibicao(image, "Imagem Original"); // passo 1
    image = TirarSombra(image);
    resize(image,image,Size(900,900));
    //exibicao(image, "resimensionada para 900x900"); // passo 2
    //exibicao(image, "Imagem sem backgound"); // passo 3
    return image; // retorna a imagem na escala de cinza
}

Mat TirarSombra2(Mat image){
    int height = image.rows;
    int width = image.cols;
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int cor = image.at<cv::Vec3b>(i,j)[0]; // blue
            int cor2 = image.at<cv::Vec3b>(i,j)[1]; //green
            int cor3 = image.at<cv::Vec3b>(i,j)[2]; // red
            if(cor > 10 && cor < 50 && cor2 > 50 && cor2 < 150 && cor3 > 70 && cor3< 210){
                image.at<cv::Vec3b>(i,j)[0]= 255;
                image.at<cv::Vec3b>(i,j)[1]= 255;
                image.at<cv::Vec3b>(i,j)[2]= 255;
            }
        }
    }
    return image;
}

Mat TirarSombra(Mat image){
    int height = image.rows;
    int width = image.cols;
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int cor = image.at<cv::Vec3b>(i,j)[0]; // blue
            int cor2 = image.at<cv::Vec3b>(i,j)[1]; //green
            int cor3 = image.at<cv::Vec3b>(i,j)[2]; // red
            if(cor > 60){
                image.at<cv::Vec3b>(i,j)[0]= 255;
                image.at<cv::Vec3b>(i,j)[1]= 255;
                image.at<cv::Vec3b>(i,j)[2]= 255;
            } else if(cor < 70 && cor2 < 70 && cor3 < 70 ){
                image.at<cv::Vec3b>(i,j)[0]= 255;
                image.at<cv::Vec3b>(i,j)[1]= 255;
                image.at<cv::Vec3b>(i,j)[2]= 255;
            }
        }
    }
    return image;
}

Mat converterRdgGrayScale(Mat image){
    cv::Mat result;
    cv::cvtColor(image, result, cv::COLOR_BGR2GRAY);
    return result;
}

Mat segmentacaoThreshold(Mat image){
    cv::threshold(image,image,120,255,cv::THRESH_BINARY);
    Mat elem1 = cv::getStructuringElement( cv::MORPH_RECT, cv::Size(10, 10)); //Elemento estruturante
    cv::imwrite("elemento1.png", elem1);
    cv::morphologyEx(image,image,cv::MORPH_OPEN, elem1);
    cv::blur(image, image,Size(20,20));
    cv::threshold(image,image,0,255,THRESH_BINARY+THRESH_OTSU);
    return image;
}

int exibicao(Mat dst, string titulo){
    //cv::namedWindow(titulo);
    //cv::imshow(titulo, dst);
    cv::imwrite(titulo , dst);
    return 0;
}

Mat contorno(Mat src){
    // extrator de contorno
    cv::Mat kernel = cv::Mat::ones(3, 3, CV_8U);
    cv::Mat eroded;
    cv::erode(src, eroded, kernel);
    cv::Mat dst = src - eroded;
    return dst;

}

Mat preenchimento(Mat src){
    cv::normalize(src, src, 0, 1, cv::NORM_MINMAX);
    cv::Mat dst;
    dst = cv::Mat::zeros(src.size(), CV_8U);
    dst.at<uchar>(28,74) = 1;
    cv::Mat prev;
    cv::Mat kernel = (cv::Mat_<uchar>(3,3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
    do {
        dst.copyTo(prev);
        cv::dilate(dst, dst, kernel);
        dst &= (1 - src);
    }
    while (cv::countNonZero(dst - prev) > 0);
    cv::normalize(src, src, 0, 255, cv::NORM_MINMAX);
    cv::normalize(dst, dst, 0, 255, cv::NORM_MINMAX);
    return dst;
}

Mat sistemaSaida(Mat fundo, Mat comprimido, Mat vazio){
    int height = fundo.rows;
    int width = fundo.cols;
    cv::Mat imagemFinal(height, width, CV_8UC3 );
    //cout << imagemFinal.channels() << endl;
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int fun = fundo.at<uchar>(i,j);
            int com = comprimido.at<uchar>(i,j);
            int sem = vazio.at<uchar>(i,j);
            if(fun == 0){
                imagemFinal.at<cv::Vec3b>(i,j)[0]= 127;
                imagemFinal.at<cv::Vec3b>(i,j)[1]= 127;
                imagemFinal.at<cv::Vec3b>(i,j)[2]= 127;
            } else if(com == 0){
                imagemFinal.at<cv::Vec3b>(i,j)[0]= 0;
                imagemFinal.at<cv::Vec3b>(i,j)[1]= 0;
                imagemFinal.at<cv::Vec3b>(i,j)[2]= 0;
                /*if(sem == 255){
                     imagemFinal.at<cv::Vec3b>(i,j)[0]= 0;
                     imagemFinal.at<cv::Vec3b>(i,j)[1]= 0;
                     imagemFinal.at<cv::Vec3b>(i,j)[2]= 255;
                }*/
            } else if(com == 255){
                imagemFinal.at<cv::Vec3b>(i,j)[0]= 0;
                imagemFinal.at<cv::Vec3b>(i,j)[1]= 255;
                imagemFinal.at<cv::Vec3b>(i,j)[2]= 0;
            }
        }
    }
    //exibicao(imagemFinal, "imagem final");
    return imagemFinal;
}

Mat program(Mat atual){
    Mat semComprimido = atual.clone();
    Mat image = converterRdgGrayScale(atual);
    Mat segmentada = segmentacaoThreshold(image);
    //exibicao(segmentada, "cartela binarizada (otsu)");
    Mat dst = contorno(segmentada);
    //exibicao(dst, "Extrator de Contornos");
    dst = preenchimento(dst);
    //exibicao(dst, "Preenchimento da cartela");
    cv::threshold(dst,dst,0,255,cv::THRESH_BINARY_INV);
    //exibicao(dst, "Invertida do preenchimento");
    Mat finalComBorda = interseccaoImagens(segmentada, dst);
    //exibicao(finalComBorda, "Interseccao entre imagens com borda");
    Mat ImageFinalSemBorda = segmentacaoThreshold(finalComBorda);
    //exibicao(ImageFinalSemBorda, "Interseccao entre imagens sem borda");
    Mat cinza = contarCartela(dst,ImageFinalSemBorda);
    //exibicao(cinza, "Imagem Final Cinza");

    semComprimido = segmentacaoThresholdSemComprimido(semComprimido);
    //exibicao(semComprimido, "Imagem Final Cinza");
    semComprimido = cartelaSemComprimido(ImageFinalSemBorda, semComprimido);
    //exibicao(dst, "fundo");
    //exibicao(segmentada, "com comprimido");
    //exibicao(semComprimido, "Sem comprimido");
    return sistemaSaida(dst, segmentada, semComprimido);
}

Mat interseccaoImagens(Mat img1, Mat img2){
    int height = img1.rows;
    int width = img1.cols;
    cv::Mat soma(height, width, CV_8U);
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int indice1 = img1.at<uchar>(i,j);
            int indice2 = img2.at<uchar>(i,j);
            if(indice1 == 0){
                  soma.at<uchar>(i,j) = 0;
            } else if(indice2 == 0){
                  soma.at<uchar>(i,j) = 0;
            } else {
                soma.at<uchar>(i,j) = 255;
            }

        }
     }
     return soma;
}

Mat converterScala(Mat binaria){
    int height = binaria.rows;
    int width = binaria.cols;
    cv::Mat imagemCinza(height, width, CV_LOAD_IMAGE_GRAYSCALE);
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
                imagemCinza.at<uchar>(i,j) = binaria.at<uchar>(i,j);
            }
        }
    return imagemCinza;
}

Mat contar(Mat cartela,  Mat comprimido){
    nObjetos = 0;
    int height = comprimido.rows;
    int width = comprimido.cols;
    CvPoint p;
    p.x = 0;
    p.y = 0;
    cv::Mat imagemCinza = converterScala(comprimido);
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int cor = imagemCinza.at<uchar>(i,j);
            int corC = cartela.at<uchar>(i,j);
            if(cor == 255 && corC ==127){
                nObjetos++;
                p.x = j;
                p.y = i;
                floodFill(imagemCinza,p,127);
            }
        }
     }
     cout << "cartela : " << nCartela << endl ;
     cout << "quantidade comprimido: " << nObjetos << endl ;
     f_out << nObjetos;
     return imagemCinza;
}

Mat contarCartela(Mat cartela, Mat comprimido){
    nCartela = 0;
    int height = cartela.rows;
    int width = cartela.cols;
    CvPoint p;
    p.x = 0;
    p.y = 0;
    cv::Mat imagemCinza = converterScala(cartela);
    for(int i = 0 ; i<height; i++){
        for(int j = 0 ; j<width ; j++){
            int cor = imagemCinza.at<uchar>(i,j);
            if(cor == 255){
                nCartela++;
                p.x = j;
                p.y = i;
                floodFill(imagemCinza,p,127);
                comprimido = contar(imagemCinza, comprimido);
            }
        }

     }
     return imagemCinza;
}


