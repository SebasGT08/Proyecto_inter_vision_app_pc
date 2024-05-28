#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

// Función para cambiar el color de las gafas
void cambiarColorGafas(Mat& imagen, const Mat& mascara, const Scalar& nuevoColor) {
    // Crear una imagen del mismo tamaño que la imagen original con el nuevo color
    Mat gafasColor = Mat::zeros(imagen.size(), imagen.type());
    gafasColor.setTo(nuevoColor, mascara);

    // Aplicar la máscara inversa a la imagen original para mantener los otros colores
    Mat imagenFondo;
    bitwise_and(imagen, imagen, imagenFondo, ~mascara);

    // Combinar las dos imágenes
    bitwise_or(gafasColor, imagenFondo, imagen);
}

int main(int argc, char* argv[]) {
    // Manejo de Video
    VideoCapture videoPrincipal("http://192.168.83.169:5000/video_feed");

    // Verificamos si el stream de video se pudo abrir
    if (!videoPrincipal.isOpened()) {
        cerr << "Error al abrir el stream de video!" << endl;
        return -1;
    }

    // Crear ventanas para mostrar los resultados
    namedWindow("Original", WINDOW_AUTOSIZE);
    namedWindow("Gafas Color Cambiado", WINDOW_AUTOSIZE);
    namedWindow("Gafas Detectadas", WINDOW_AUTOSIZE);

    Mat frameColor;
    Mat imgHSV;
    Mat mascara;

    // Valores HSV para detectar el color magenta
    int hmin = 140, hmax = 170, smin = 100, smax = 255, vmin = 150, vmax = 255;

    // Color al que se cambiarán las gafas (rojo en este caso, en espacio BGR)
    Scalar nuevoColor(0, 0, 255);

    // Bucle principal
    while (true) {
        videoPrincipal >> frameColor; // Capturar frame del video principal
        if (frameColor.empty()) break; // Salir si no hay más frames

        // Convertir a HSV
        cvtColor(frameColor, imgHSV, COLOR_BGR2HSV);

        // Crear una máscara basada en el rango HSV
        inRange(imgHSV, Scalar(hmin, smin, vmin), Scalar(hmax, smax, vmax), mascara);

        // Cambiar el color de las gafas en la imagen
        Mat gafasColorCambiado = frameColor.clone();
        cambiarColorGafas(gafasColorCambiado, mascara, nuevoColor);

        // Extraer las gafas detectadas en una imagen separada
        Mat gafasDetectadas = Mat::zeros(frameColor.size(), frameColor.type());
        frameColor.copyTo(gafasDetectadas, mascara);

        // Convertir gafasColorCambiado y gafasDetectadas de vuelta a BGR para mostrar correctamente
        Mat gafasColorCambiadoBGR, gafasDetectadasBGR;
        cvtColor(gafasColorCambiado, gafasColorCambiadoBGR, COLOR_BGR2RGBA);
        cvtColor(gafasDetectadas, gafasDetectadasBGR, COLOR_BGR2RGBA);

        // Mostrar las imágenes en las ventanas
        imshow("Original", frameColor);
        imshow("Gafas Color Cambiado", gafasColorCambiadoBGR);
        imshow("Gafas Detectadas", gafasDetectadasBGR);

        // Salir si se presiona la tecla ESC
        if (waitKey(1) == 27) break;
    }

    // Liberar el video y destruir todas las ventanas
    videoPrincipal.release();
    destroyAllWindows();

    return 0;
}
