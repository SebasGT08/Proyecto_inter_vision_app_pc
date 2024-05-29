#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

void eventoTrack(int valor, void *data) {}

// Funcion para cambiar el color de las gafas
void cambiarColorGafas(Mat& imagen, const Mat& mascara, const Scalar& nuevoColor) {
    // Crear una imagen del mismo tamano que la imagen original con el nuevo color
    Mat gafasColor = Mat::zeros(imagen.size(), imagen.type());
    gafasColor.setTo(nuevoColor, mascara);

    // Aplicar la mascara inversa a la imagen original para mantener los otros colores
    Mat imagenFondo;
    bitwise_and(imagen, imagen, imagenFondo, ~mascara);

    // Combinar las dos imagenes
    bitwise_or(gafasColor, imagenFondo, imagen);
}

// Función para detección de bordes usando Canny
void deteccionBordes(const Mat &imagen, Mat &bordes) {
    Mat imagen_gris;
    cvtColor(imagen, imagen_gris, COLOR_BGR2GRAY);
    Canny(imagen_gris, bordes, 50, 150);
    // cvtColor(bordes, bordes, COLOR_GRAY2BGR); // Convertir a BGR para que coincida el tipo
}

int valor_b = 0;
int valor_g = 0;
int valor_r = 255;

int main(int argc, char* argv[]) {
    // Capturar Video
    VideoCapture videoPrincipal("http://192.168.18.250:5000/video_feed");

    // Verificamos si el stream se puede abrir
    if (!videoPrincipal.isOpened()) {
        cerr << "Error al abrir el stream de video!" << endl;
        return -1;
    }

    // Crear ventanas para mostrar los resultados
    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("Gafas Bordes", WINDOW_AUTOSIZE);
    namedWindow("Gafas Color Cambiado", WINDOW_AUTOSIZE);
    namedWindow("Gafas Detectadas", WINDOW_AUTOSIZE);

    // Crear trackbars para elegir el color de las gafas (se cambia el nombre de BRG A RGB por que se tranforma depues)
    createTrackbar("R", "Gafas Color Cambiado", &valor_b, 255, eventoTrack, NULL);
    createTrackbar("G", "Gafas Color Cambiado", &valor_g, 255, eventoTrack, NULL);
    createTrackbar("B", "Gafas Color Cambiado", &valor_r, 255, eventoTrack, NULL);

    Mat frameOriginal;
    Mat imgHSV;
    Mat mascara;

    // Valores HSV para detectar el color magenta
    int hmin = 140, hmax = 170, smin = 100, smax = 255, vmin = 150, vmax = 255;

    // Bucle principal
    while (true) {
        videoPrincipal >> frameOriginal; // Capturar frame del video principal
        if (frameOriginal.empty()) break; // Salir si no hay más frames

        // Convertir a HSV
        cvtColor(frameOriginal, imgHSV, COLOR_BGR2HSV);

        // Crear una máscara basada en el rango HSV
        inRange(imgHSV, Scalar(hmin, smin, vmin), Scalar(hmax, smax, vmax), mascara);

        // Actualizar el color de las gafas usando los valores de los trackbars
        Scalar nuevoColor(valor_b, valor_g, valor_r);

        // Cambiar el color de las gafas en la imagen
        Mat gafasColorCambiado = frameOriginal.clone();
        cambiarColorGafas(gafasColorCambiado, mascara, nuevoColor);

        // Extraer las gafas detectadas en una imagen separada
        Mat gafasDetectadas = Mat::zeros(frameOriginal.size(), frameOriginal.type());
        frameOriginal.copyTo(gafasDetectadas, mascara);

        //Deteccion de bordes
        Mat bordes_canny;
        deteccionBordes(gafasDetectadas, bordes_canny);

        //Convertir a RGB
        Mat gafasColorCambiadoRGB;
        cvtColor(gafasColorCambiado, gafasColorCambiadoRGB, COLOR_BGR2RGBA);

        // Mostrar las imágenes en las ventanas
        imshow("Original", frameOriginal);
        imshow("Gafas Bordes", bordes_canny);
        imshow("Gafas Color Cambiado", gafasColorCambiadoRGB);
        imshow("Gafas Detectadas", gafasDetectadas);

        // Salir si se presiona la tecla ESC
        if (waitKey(1) == 27) break;
    }

    // Liberar el video y destruir todas las ventanas
    videoPrincipal.release();
    destroyAllWindows();

    return 0;
}
