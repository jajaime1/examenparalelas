#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <cstring>
#include <cmath>
#include <omp.h>
#include <wchar.h>
#include <regex>

using namespace std;

// Función que convierte una fecha en segundos desde la época
double fecha_a_segundos(const char* anio, const char* mes, const char* dia) {
  tm fecha;
  fecha.tm_year = atoi(anio) - 1900;;
  fecha.tm_mon = atoi(mes) - 1;
  fecha.tm_mday = atoi(dia);
  time_t t = mktime(&fecha);
  return t;
}

bool validar_fecha(const string& fecha){
    regex formato("\\d{4}-\\d{2}-\\d{2}$");
    if (!regex_match(fecha, formato)){
        return false;
    }

    int year, month, day;
    sscanf(fecha.c_str(), "%d-%d-%d", &year, &month, &day);

    if (year < 1900){
        return false;
    }
    if (month < 1 || month > 12){
        return false;
    }
    if (day < 1 || day > 31){
        return false;
    }
}

int main()
{

        // Ingresamos la fecha manualmente

    string fecha_str;

    while (true){
        cout << "Ingrese una fecha en formato (YYYY-MM-DD): ";
        cin >> fecha_str;

        if (validar_fecha(fecha_str)){
            break;
        }else {
            cout << "Por favor, ingrese una fecha válida en formato (YYYY-MM-DD)." << endl;
        }
    }

    //verificamos si los componentes son validos
    int year, month, day;
    sscanf(fecha_str.c_str(), "%d-%d-%d", &year,&month, &day);

    // Convertimos la fecha ingresada a segundos desde la época
    double fecha_segundos = fecha_a_segundos(to_string(year).c_str(), to_string(month).c_str(), to_string(day).c_str());

    // Obtenemos la fecha actual en segundos desde la época
    time_t now = time(0);
    tm* ltm = localtime(&now);
    double fecha_actual_segundos = fecha_a_segundos(to_string(ltm->tm_year + 1900).c_str(), to_string(ltm->tm_mon + 1).c_str(), to_string(ltm->tm_mday).c_str());

    // Calculamos la edad en años
    double edad_anios = (fecha_actual_segundos - fecha_segundos) / (365.25 * 24 * 60 * 60);

    setlocale(LC_ALL, ""); // del wchar.h para usar caracteres como ó

    // Verificamos el grupo etario
    string grupo_etario;
    if (edad_anios < 5) {
        grupo_etario = "Bebes";
    } else if (edad_anios >= 5 && edad_anios < 12) {
        grupo_etario = "Niños";
    } else if (edad_anios >= 12 && edad_anios < 18) {
        grupo_etario = "Adolescentes";
    } else if (edad_anios >= 18 && edad_anios < 25) {
        grupo_etario = "Jóvenes";
    } else if (edad_anios >= 25 && edad_anios < 40) {
        grupo_etario = "Adultos jóvenes";
    } else if (edad_anios >= 40 &&edad_anios < 55) {
        grupo_etario = "Adultos";
    } else if (edad_anios >= 55 &&edad_anios < 65) {
        grupo_etario = "Adultos mayores";
    } else if (edad_anios >= 65 &&edad_anios < 75) {
        grupo_etario = "Ancianos";
    } else {
        grupo_etario = "Longevos";
    }

    // Mostramos el resultado
    cout << "La fecha ingresada corresponde al grupo etario: " << grupo_etario << endl;



    // Mostramos el resultado del grupo de edad correspondiente a la fecha ingresada
    cout << "\nResultado para la fecha " << fecha_str << ":" << endl;
    cout << "Grupo etario\t" << grupo_etario << endl;

    ifstream Archivo;
    Archivo.open("datosdeprueba.csv");
    string linea = "";

    // Variables para contar el número de personas en cada grupo de edad
    int bebes = 0;
    int ninos = 0;
    int adolescentes = 0;
    int jovenes = 0;
    int adultos_jovenes = 0;
    int adultos = 0;
    int adultos_mayores = 0;
    int ancianos = 0;
    int longevos = 0;
    int total_poblacion = 0;

    getline(Archivo, linea); // Salta la primera línea
    omp_set_num_threads(6); // Definimos el número de hilos a 6

    const int BATCH_SIZE = 1000; // Número de líneas a procesar en cada lote

    // Variable sec para almacenar la fecha en segundos desde la época
    double sec = 0.0;

    // Bucle externo para leer lotes de líneas del archivo
    while (!Archivo.eof()) {
        vector<string> lineas_lote; // Almacena las líneas leídas en el lote

        // Leer un lote de líneas del archivo
        for (int i = 0; i < BATCH_SIZE && getline(Archivo, linea); i++) {
            lineas_lote.push_back(linea);
        }

        // Bucle interno para procesar cada línea dentro del lote
        #pragma omp parallel for reduction(+:total_poblacion, bebes, ninos, adolescentes, jovenes, adultos_jovenes, adultos, adultos_mayores, ancianos, longevos)
        for (int i = 0; i < lineas_lote.size(); i++) {
            string linea = lineas_lote[i];

            string fecha;
            string tempString = "";
            stringstream inputString(linea);
            getline(inputString, fecha, ';');

            // Dividimos la fecha en componentes utilizando strtok()
            char* date_str = strdup(fecha.c_str());
            char* day_str = strtok(date_str, "-");
            char* month_str = strtok(NULL, "-");
            char* year_str = strtok(NULL, "-");

            // Convertimos la fecha a segundos desde la época
            if (i == 0) {
                sec = fecha_a_segundos(year_str, month_str, day_str);
            }

            // Calculamos la edad en años
            time_t now = time(0);
            tm* ltm = localtime(&now);

            int edad = ltm->tm_year + 1900 - atoi(year_str);

            // Verifica si ya pasó el cumpleaños de este año
            if (ltm->tm_mon + 1 < atoi(month_str) || (ltm->tm_mon + 1 == atoi(month_str) && ltm->tm_mday < atoi(day_str))) {
                edad--;
            }

            // Incrementamos el contador correspondiente al grupo de edad
            #pragma omp critical
            {
                total_poblacion++;
                if (edad < 5) {
                    bebes++;
                } else if (edad < 12) {
                    ninos++;
                } else if (edad < 18) {
                    adolescentes++;
                } else if (edad < 25) {
                    jovenes++;
                } else if (edad < 40) {
                    adultos_jovenes++;
                } else if (edad < 55) {
                    adultos++;
                } else if (edad < 65) {
                    adultos_mayores++;
                } else if (edad < 75) {
                    ancianos++;
                } else {
                    longevos++;
                }
            }
        }
    }

    setlocale(LC_ALL, ""); // del wchar.h para usar caracteres como ó

    // Calculamos y mostramos el porcentaje de la población en cada grupo de edad
    cout << "\nPorcentaje de la población en cada grupo de edad:\n" << endl;
    cout << "Bebes\t\t" << (float)bebes / total_poblacion * 100 << " %\n" << endl;
    cout << "Niños\t\t" << (float)ninos / total_poblacion * 100 << " %\n" << endl;
    cout << "Adolescentes\t" << (float)adolescentes / total_poblacion * 100 << " %\n" << endl;
    cout << "Jóvenes\t\t" << (float)jovenes / total_poblacion * 100 << " %\n" << endl;
    cout << "Adultos jóvenes\t" << (float)adultos_jovenes / total_poblacion * 100 << " %\n" << endl;
    cout << "Adultos\t\t" << (float)adultos / total_poblacion * 100 << " %\n" << endl;
    cout << "Adultos mayores\t" << (float)adultos_mayores / total_poblacion * 100 << " %\n" << endl;
    cout << "Ancianos\t" << (float)ancianos / total_poblacion * 100 << " %\n" << endl;
    cout << "Longevos\t" << (float)longevos / total_poblacion * 100 << " %\n" << endl;


    return 0;
}
