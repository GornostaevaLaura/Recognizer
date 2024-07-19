// диплом1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//


#include <iostream>
#include <stdlib.h>
#include <complex>
#include <vector>
#include <fstream>
#include <cmath>
//#include <math.h>
#include <cstring>
//#include <ctime>
#include <cstdlib>
#include <algorithm>


#define PI 3.1415926535
#define M 400				//количество окон
//#define N 409			//количество отсчетов
#define CofMFCC 12  //исправить на 2...13 включительно

#define Fs 400
#define fmin 400
#define fmax 400
#define maxSlovCom 3
#define minSlovCom 2

#define sizeFrame 20.0   ///мс
#define stepFrame 10     ///мс

using namespace std;

typedef complex <double> base;

double powerO[M] = { 0 };			//логарифмическая энергия для каждого окна 

int const DIM1 = 17;        // Количество различных слов
int const DIM2 = DIM1;
int const DIM3 = 4;         // 4 - максимальное число команд, содержащих данных переход
int const DIM4 = 34;        // Количество команд

// Структура, описывающая заголовок WAV файла.
struct WAVHEADER
{
    char chunkId[4];    //RIFF-заголовок:
    unsigned long chunkSize;    // размер файла

    char format[4];
    char subchunk1Id[4];    // "fmt "
    unsigned long subchunk1Size;    // 16 для формата PCM.

    // Для PCM = 1 (то есть, Линейное квантование).
    // Значения, отличающиеся от 1, обозначают некоторый формат сжатия.
    unsigned short audioFormat;
    unsigned short numChannels;     // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    unsigned long sampleRate;       // Частота дискретизации.

    // sampleRate * numChannels * bitsPerSample/8
    unsigned long byteRate;

    // numChannels * bitsPerSample/8
    // Количество байт для одного сэмпла, включая все каналы.
    unsigned short blockAlign;

    // Так называемая "глубиная" или точность звучания. 8 бит, 16 бит и т.д.
    unsigned short bitsPerSample;

    //__________________________________________________________________________
    char subchunk2ID[4];        // "data"

    // numSamples * numChannels * bitsPerSample/8
    // Количество байт в области данных.
    unsigned long subchunk2Size;

    // Далее следуют непосредственно Wav данные.
};

//Структура, описывающая наличие данных о пользователе в системе
struct userInf {
    int userID{};
    string name;

    //номер команды, если ли доступ, есть ли данные
    vector <vector <int>> comInf; 
};

//Структура, описывающая данные о пользователе (по словам)
struct userData {

    int userId{};
    string slovo;
    double slovData[CofMFCC];
};

int main()
{
    setlocale(LC_ALL, "RUSSIAN");

    // создание трехмерной матрицы перехода

    //vector < vector < vector <int> > > MatrCom;
    int*** MatrCom;
    MatrCom = new int** [DIM1];
    for (int i = 0; i < DIM1; i++) {
        MatrCom[i] = new int* [DIM2];
        for (int j = 0; j < DIM2; j++) {

            MatrCom[i][j] = new int[DIM3];
            memset(MatrCom, 0, DIM3 * sizeof(int));       //обнуление
        }
    }

    initMatrix(MatrCom);            //инициализация матрицы перехода, определяющей произнесенную команду

    //Список пользователей (ID, name)
    vector <vector <string> > users;

    // Хранение данных о пользователях
    vector <userInf> inftable;

    // Список данных о словах для каждого пользователя
    vector <userData> datatable;
    // 
    //МЕНЮ
    //______________________________________________________________________________
    int mod = 0;
    while (true)
    {
        cout << " 0. СПРАВКА \n 1. Внесение данных в систему\n 2. Выполнение команды\n 3. Выход\n";
        cin >> mod;
        if (mod == 0)
        {
            // ВЫВОД ДАННЫХ, СОДЕРЖАЩИХСЯ В СИСТЕМЕ - vector <userInf> inftable

            if (!inftable.empty())
            {

                cout << "|Имя пользователя \t|| Номер команды \t|| Наличие доступа \t|| Наличие эталонных данных|\n";
                cout << "___________________________________________________________________________________________________\n";
                for (int i = 0; i < inftable.size(); i++)
                {
                    for (int j = 0; j < inftable[i].comInf.size(); j++)
                    {
                         cout << "|" << inftable[i].name << " \t|| " << inftable[i].comInf[j][0] << " \t|| ";
                         if (inftable[i].comInf[j][1] == (int)true)
                             cout << "Да \t||";
                         else cout << "Нет \t||";


                         if (inftable[i].comInf[j][2] == (int)true)
                             cout << "Да \t|\n";
                         else cout << "Нет \t|\n";

                         cout << "___________________________________________________________________________________________________\n";
                    }
                       
                }


            }
            else cout << "Система не содержит данных\n";
        }
        else if (mod == 1)
        {
            //ПОЛУЧЕНИЕ ЭТАЛОННЫХ ЗНАЧЕНИЙ
            string nameus;
            int countsl = 0, numbcom = 0;
           // cout << " *****ВНЕСЕНИЕ ПОЛЬЗОВАТЕЛЯ В СИСТЕМУ***** \n";
            cout << " Введите имя пользователя: \n";
            cin >> nameus;

            // Проверка наличие данного пользователя в системе
            int usid = -1, nUser;
            for (int i = 0; i < inftable.size(); i++)
            {
                if (inftable[i].name == nameus)
                    usid = inftable[i].userID;
                nUser = i;
            }

            if (usid == -1)
            {
                // Такого пользователя в системе нет. Добавление нового
                cout << "\n *****ВНЕСЕНИЕ ПОЛЬЗОВАТЕЛЯ В СИСТЕМУ***** \n";
                userInf usNEW;
                nUser = inftable.size();
                usid = nUser + 1;
                usNEW.userID = usid;
                usNEW.name = nameus;
                inftable.push_back(usNEW);
                

            }
            else
            {
                // Такой пользователь присутствует в системе
                cout << "\n Данный пользователь присутствует в системе. ***РЕДАКТИРОВАНИЕ ДАННЫХ О ПОЛЬЗОВАТЕЛЕ " << nameus << " ***\n";
            }

            while (true)
            {
                cout << "\n Введите номер команды: \n";
                cin >> numbcom;

                if ((numbcom < 1) || (numbcom > DIM4))
                    cout << "ОШИБКА!!! Команды с таким номером нет\n";
                else break;
            }

                int choice;
                bool addCom = true;
                for (int i = 0; i < inftable[nUser].comInf.size(); i++)
                {
                    if (inftable[nUser].comInf[i][0] == numbcom)        //Данная команда есть в информации о пользователе
                    {
                        addCom = false;
                        if (inftable[nUser].comInf[i][1] == (int)true)
                        {
                            while (true)
                            { 
                                cout << "У пользователя есть права на эту команду. Удалить доступ к данной команде? 0 - нет, 1 - да\n";
                            
                                cin >> choice;
                                if (choice == 1)
                                {
                                    inftable[nUser].comInf[i][1] == (int)false;
                                    break;
                                }
                                else if (choice == 0)
                                    break;
                                else cout << "Такой команды нет\n";
                            }

                        }
                        else inftable[nUser].comInf[i][1] = (int)true;

                        if (inftable[nUser].comInf[i][2] == (int)false)
                        {
                            cout << "У пользователя отсутствуют параметры голоса для данной команды. Осуществить ввод соответствующих данных? 0 - нет, 1 - да\n";
                            cin >> choice;
                            if (choice == 1)
                            {
                                //ВВОД ПАРАМЕТРОВ ГОЛОСА
                                gettingData(numbcom, usid, datatable);

                                inftable[nUser].comInf[i][3] = (int)true;

                            } else if(choice != 0)
                                cout << "Такой команды нет\n";
                        } else cout << "У пользователя добавлены параметры голоса для данной команды\n";

                        break;
                    }

                }
                //Добавление информации о команде // Добавить права на команду Добавить данные о команде
                if (addCom)
                {
                    cout << " *** ДОБАВЛЕНИЕ ПОЛЬЗОВАТЕЛЮ ДОСТУПА К ДАННОЙ КОМАНДE ***\n ";
                    vector <int> flagCom;
                    flagCom.push_back(numbcom);
                    flagCom.push_back(int(true));           //наличие доступа к команде

                    //ВВОД ПАРАМЕТРОВ ГОЛОСА
                    gettingData(numbcom, usid, datatable);

                    flagCom.push_back(int(true));// наличие данных (параметры голоса для данной команды)

                    inftable[nUser].comInf.push_back(flagCom);

                }
                    

        }
        else if (mod == 2)
        {
            //ОПРЕДЕЛЕНИЕ КОМАНДЫ
            char defCommand[DIM4] = { 0 };          //Массив для определения команды по матрице переходов

            //Чтение данных

            WAVHEADER header;
            FILE* infile = readf(header);

            double* indata = new double[header.subchunk2Size];
            int prov = fread_s(&indata, sizeof(header.subchunk2Size), sizeof(header.subchunk2Size), 1, infile);
            if (!feof(infile))                   //читаем до конца
            {
                cout << " Конец файла не достигнут. Считанно " << prov << endl;
            }
            else cout << "Данные из файла успешно считаны\n";

            //___________________________________________________________________________________________________________________________________________________
            //Выделение признаков для каждого слова
            /// дробление по словам и определение их количества

            vector <vector <double>> SLdata;                        //Данные из записи разбитые по словам


             // Дробление по словам
            splittingWords(header, SLdata, indata);

            char slovCount = SLdata.size(); /// количество получившихся слов


            //Правильно ли поделилось
            if ((slovCount < minSlovCom) || (slovCount > maxSlovCom))
            {
                cout << "ОШИБКА выделения слов\n";
                return;
            }

            /// Деление каждого слова на фреймы





            ///??????????





            int last = -1, out = -1;
            string slovo;
            int user;

            for (int i = 0; i < slovCount; i++)  //Цикл по каждому слову
            {
                double kof[CofMFCC] = { 0 };
                /// Деление каждого слова на фреймы
                vector <vector <double>> FrameSL;
                int c = framesDivision(header, SLdata[i], FrameSL);         //количество отсчетов в фрейме



                //___________________________________________________________________________________________________________________________________________________
                //Определение произнесенного слова и говорящего

                

                // ПОЛУЧЕНИЕ MVCC КОЭФФИЦИЕНТОВ ДЛЯ СЛОВА для каждого фрейма?

                vector <vector <base>> x;
                //base* x = new base [SLdata[i].size()];
               /* for (int j = 0; j < SLdata[i].size(); j++)
                {
                    x.push_back(complex <double>(SLdata[i][j], 0.0));
                }
                */
                for (int j = 0; j < FrameSL.size(); j++)              //проход по каждому фрейму
                {
                    for (int k = 0; k < FrameSL[j].size(); k++)              //проход по всем значениям
                    {
                        x[j].push_back(complex <double>(FrameSL[j][k], 0.0));
                    }

                    FFT(x[j]);     //преобразование фурье
                    
                    getMFCCkoef(x, c, FrameSL.size(), kof);           //определение коэффициентов: входные данные, размерность, количество окон, вектор для записи коэф

                }
                
                cout << "Коэффициенты MFCC для " << i + 1 << "слова:    \n";
                for (int k = 0; k < CofMFCC; k++)
                    cout << kof[i] << "  ";
                cout << endl;

                
                /////       КЛАССИФИКАТОР
                if (datatable.empty())
                {
                    cout << " В системе нет данных о пользователях\n";
                    continue;
                }
                int res = classification(kof, datatable);
                

                slovo = datatable[res].slovo;
                user = datatable[res].userId;


                //Определение значения соответствующего слова в матрице перехода 
                /* Количество всех возможных слов - 17. 0 - включить, 1 - отключить, 2 - выключить, 3 - заблокировать, 4 - разблокировать
                5 - свет, 6 - климат-контроль, 7 - розетки, 8 - телевизор, 9 - входную, 10 - дверь, 11 - окно, 12 - на кухне, 13 - в спальне, 14 - в гостиной, 15 - в детской, 16 - на балкон*/
                if (slovo == "включить") out = 0;
                else if (slovo == "отключить") out = 1;
                else if (slovo == "выключить") out = 2;
                else if (slovo == "заблокировать") out = 3;
                else if (slovo == "разблокировать") out = 4;
                else if (slovo == "свет") out = 5;
                else if (slovo == "климат-контроль") out = 6;
                else if (slovo == "розетки") out = 7;
                else if (slovo == "телевизор") out = 8;
                else if (slovo == "входную") out = 9;
                else if (slovo == "дверь") out = 10;
                else if (slovo == "окно") out = 11;
                else if (slovo == "на кухне") out = 12;
                else if (slovo == "в спальне") out = 13;
                else if (slovo == "в гостиной") out = 14;
                else if (slovo == "в детской") out = 15;
                else if (slovo == "на балкон") out = 16;

                if (out >= 0)
                {
                    cout << "\nОпределено слово: " << slovo << ", ему присвоенно значение = " << out << endl;
                }
                else cout << "\nОшибка! Слову: " << slovo << " не присвоенно значение!\n ";
                //___________________________________________________________________________________________________________________________________________________
                //Нарощение команды. Определение к какой команде относится переход

                if (last != -1)
                {                    
                    phraseCommands(defCommand, MatrCom, last, out); // определение, каким командам соответствует переход last->out
                }                   
                last = out;
            }
            
            char command;
            //Определение номера окончательной команды
            for (int i = 0; i < DIM4; i++)
                if (defCommand[i] == slovCount - 1)
                {
                    command = i;
                    break;
                }
            cout << "*****РЕЗУЛЬТАТ*****  Пользователь " << user << ", номер команды - " << command << endl;

            // Проверка доступа
            bool flag = false;

            for (int i = 0; i < inftable.size(); i++)
            {
                if (inftable[i].userID == user)
                {
                    for (int j = 0; j < inftable[i].comInf.size(); j++)
                        if (inftable[i].comInf[j][0] == command)
                        {
                            flag = true;
                            break;
                        }
                            
                    break;
                }
                    
            }

            if (flag)
                cout << "************* ДОСТУП РАЗРЕШЕН *************\n";
            else cout << "************* в ДОСТУПЕ ОТКАЗАНО *************\n";

        }
        else if (mod == 3)
        {
            
            // DELETE!!!!!!!

            for (int i = 0; i < DIM1; i++)  
                delete [] MatrCom[i];
                
            

            return 0;


        }
        else cout << "Такой команды нет\n";
    }
    
  

	
}

double windowsF(double x, int t)            //окно Хэмминга
{
    double arg = 2 * PI * t / sizeFrame;                     //double arg = 2.0 * PI * t * PI / (sizeFrame * 180);
    arg = cos(arg);
    double w = 0.53836 - (0.46164 * arg);                  //в радианах    //Один радиан эквивалентен 180/PI градусам.
   
    return w * x;
}

//Функция быстрого преобразования Фурье 
void FFT(vector<base>& a)
{
    int n = (int)a.size();
    if (n == 1)  return;

    vector<base> a0(n / 2), a1(n / 2);
    for (int i = 0, j = 0; i < n; i += 2, ++j)
    {
        a0[j] = a[i];
        a1[j] = a[i + 1];
    }
    FFT(a0);
    FFT(a1);

    double ang = 2 * PI / n;
    base w(1), wn(cos(ang), sin(ang));
    for (int i = 0; i < n / 2; ++i)
    {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        w *= wn;
    }
}
//Функция получения MFCC-коэффициентов 
void getMFCCkoef(vector <vector<base>> X, int N, int countFrame, double C[])
{
	for (int m = 0; m < countFrame; m++)
	{
		double sum = 0;
		for (double h = 0, int k = 0; k < N; k++)
		{
			h = Hm(k, m);
			sum += pow(abs(X[m][k]), 2) * h;
		}
		powerO[m] = log(sum);
	}
	DiskCosP(C);
}
double Hm(int k, int m)
{
	double fm1 = F(m - 1);
	double fm2 = F(m);
	double fm3 = F(m + 1);
	double h = 0;

	if (k >= fm2)
	{
		if (k <= fm3)
			h = (fm3 - k) / (fm3 - fm2);
	}
	else if (k >= fm1)
		h = (k - fm1) / (fm2 - fm1);
}
double F(int m)
{
	return (M / Fs) * melf(mel(fmin) + m * mel(fmax - fmin) / (M + 1));
}

const double koefMel = 1127.01048;
double mel(double f)
{
	return koefMel * log(1 + (f / 700));
}
double melf(double mel)
{
	return 700 * (exp(mel / koefMel) - 1);
}

const int countKoef = 12;
void DiskCosP(double C[])
{
	for (int n = 0; n <= countKoef; n++)
	{
		double sum = 0;
		for (int m = 0; m < M; m++)
			sum += powerO[m] * cos(PI * n * (m + 0.5) / M);

		C[n] = sum;
	}
}

FILE *readf(WAVHEADER header)
{
    string namefile;
    cout << "\n Введите имя файла с голосом пользователя: \n";
    cin >> namefile;

    // std::ifstream in(namefile);
    FILE* infile;
    errno_t error;
    //double err = -5;
    error = fopen_s(&infile, namefile.data(), "rb");
    if (error)
    {
        printf_s("\n Failed open file\n");
        
        return;
    }

    //WAVHEADER header;
    fread_s(&header, sizeof(WAVHEADER), sizeof(WAVHEADER), 1, infile);
    // Выводим полученные данные
    cout << "\n  Считывание заголовка WAV-файла\n-------------------------------------------------\n";
    cout << " Инфо о формате: " << header.chunkId[0] << header.chunkId[1] << header.chunkId[2] << header.chunkId[3] << endl;
    cout << "Оставшийся размер файла начиная с этой позиции: " << header.chunkSize << endl;
    cout << " Формат данных: " << header.format[0] << header.format[1] << header.format[2] << header.format[3] << endl;
    cout << " Параметы WAV-файла: " << header.subchunk1Id[0] << header.subchunk1Id[1] << header.subchunk1Id[2] << header.subchunk1Id[3] << endl;
    cout << " subchunk1Size (16 для pcm Это оставшийся размер подцепочки, начиная с этой позиции 35-19=16): " << header.subchunk1Size << endl;
    cout << " audioFormat: (для pcm =1) " << header.audioFormat << endl;
    cout << " Каналов: " << header.numChannels << endl;
    if (header.numChannels == 2)
    {
        cout << " => Стерео \n";
    }
    else
    {
        cout << " => Моно \n";
    }
    cout << " Частота дискретизации:" << header.sampleRate << endl;
    //unsigned long temp = header.SamplesRate*header.nChannels*header.BitsPerSample / 8;
    cout << " Байт в секунду: " << header.byteRate << endl;
    cout << " Размер blockAlign:  " << header.blockAlign << endl;
    cout << " Размер сэмпла (кол-во бит в сэмпле (глубина) 8/16: " << header.bitsPerSample << endl;
    cout << " Subchunk2ID:  " << header.subchunk2ID[0] << header.subchunk2ID[1] << header.subchunk2ID[2] << header.subchunk2ID[3] << endl;
    cout << " SubChunk2Size: " << header.subchunk2Size << endl;

    // Длительность воспроизведения в секундах
    float fDurationSeconds = 1.f * header.chunkSize / (header.bitsPerSample / 8) / header.numChannels / header.sampleRate;
    int iDurationMinutes = (int)floor(fDurationSeconds) / 60;
    fDurationSeconds = fDurationSeconds - (iDurationMinutes * 60);
    cout << " Продолжительность: " << iDurationMinutes << "." << (int)fDurationSeconds << endl;

    return infile;
}

int gettingData(int nCom, int usId, vector <userData> datatable) //Внесение данных о голосе             ||vector <userInf> inftable, vector <userData> datatable
{
    //ВВОД ПАРАМЕТРОВ ГОЛОСА
    WAVHEADER header;
    FILE* infile = readf(header);

    double* indata = new double[header.subchunk2Size];
    int prov = fread_s(&indata, sizeof(header.subchunk2Size), sizeof(header.subchunk2Size), 1, infile);
    if (!feof(infile))                   //читаем до конца
    {
        cout << " Конец файла не достигнут. Считанно " << prov << endl;
    }
    else cout << "Данные пользователя из файла успешно считаны\n";
   
    //СОЗДАНИЕ ЭТАЛОННЫХ ЗНАЧЕНИЙ

    int countsl;
    if ((nCom > 0) && (nCom < 33))
    {
        countsl = 3;
    }
    else if ((nCom == 33) || (nCom == 34))
    {
        countsl = 2;
    }
    string* slovCom = new string[countsl];


    vector <vector <double>> SLdata;                        //Данные из записи разбитые по словам
   // vector <double> sld;                                    //Данные одного слова
   // sld.push_back(-5.0);
   // SLdata.push_back(sld);


    
    //_________________________________________________________________________________________________________________________________________________
    // выделили слово, по номеру команды определили какое это слово, занесли посчитанные параметры в таблицу
    // пользователь (string) | слово (string) | параметры

    // Дробление по словам
    splittingWords(header, SLdata, indata);

    //Правильно ли поделилось
    if (SLdata.size() != countsl)
    {
        cout << "ОШИБКА выделения слов\n";
        return;
    }

    //деление слова на фреймы

    ///???????




    // Определенение распознаных слов по известному номеру команды

     /* Количество возможных команд - 34: 1. Вкл свет в гостиной,    2. Выкл свет в гостиной,   3. Вкл свет в спальне,   4. Выкл свет в спальне, 5. Вкл свет на кухне,   6. Выкл свет на кухне,
    7. Вкл свет в детсткой, 8. Выкл свет в детской, 9. Вкл климат-контроль в гостиной, 10. Выкл климат-контроль в гостиной, 11. Вкл климат-контроль в спальне,  12. Выкл климат-контроль в спальне,
    13. Вкл климат-контроль на кухне,   14. Выкл климат-контроль на кухне,  15. Вкл климат-контроль в детской,  16. Выкл климат-контроль в детской, 17. Заблокировать входную дверь,    18. Разблокировать входную дверь,
    19. Заблокировать окно в детской,   20. Разблокировать окно в детской,  21. Заблокировать окно в гостиной, 22. Разблокировать окно в гостиной,  23. Заблокировать дверь в спальне,  24. Разблокировать дверь в спальне,
    25. Заблокировать дверь на балкон,  26. Разблокировать дверь на балкон, 27. Включить розетки на кухне, 28. Отключить розетки на кухне,  29. Включить розетки в гостиной,    30. Отключить розетки в гостиной,
    31. Включить розетки в детской, 32. Отключить розетки в детской,    33. Вкл телевизор,  34. Выкл телевизор */
    
    switch (nCom)
    {
        case 1:         
            slovCom[0] = "включить";
            slovCom[1] = "свет";
            slovCom[2] = "в гостиной";
            break;
        case 2:
            slovCom[0] = "выключить";
            slovCom[1] = "свет";
            slovCom[2] = "в гостиной";
            break;

        case 3:
            slovCom[0] = "включить";
            slovCom[1] = "свет";
            slovCom[2] = "в спальне";
            break;
        case 4:
            slovCom[0] = "выключить";
            slovCom[1] = "свет";
            slovCom[2] = "в спальне";
            break;

        case 5:
            slovCom[0] = "включить";
            slovCom[1] = "свет";
            slovCom[2] = "на кухне";
            break;
        case 6:
            slovCom[0] = "выключить";
            slovCom[1] = "свет";
            slovCom[2] = "на кухне";
            break;

        case 7:
            slovCom[0] = "включить";
            slovCom[1] = "свет";
            slovCom[2] = "в детской";
            break;
        case 8:
            slovCom[0] = "выключить";
            slovCom[1] = "свет";
            slovCom[2] = "в детской";
            break;

        case 9:
            slovCom[0] = "включить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в гостиной";
            break;
        case 10:
            slovCom[0] = "выключить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в гостиной";
            break;

        case 11:
            slovCom[0] = "включить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в спальне";
            break;
        case 12:
            slovCom[0] = "выключить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в спальне";
            break;

        case 13:
            slovCom[0] = "включить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "на кухне";
            break;
        case 14:
            slovCom[0] = "выключить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "на кухне";
            break;

        case 15:
            slovCom[0] = "включить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в детской";
            break;
        case 16:
            slovCom[0] = "выключить";
            slovCom[1] = "климат-контроль";
            slovCom[2] = "в детской";
            break;

        case 17:
            slovCom[0] = "заблокировать";
            slovCom[1] = "входную";
            slovCom[2] = "дверь";
            break;
        case 18:
            slovCom[0] = "разблокировать";
            slovCom[1] = "входную";
            slovCom[2] = "дверь";
            break;

        case 19:
            slovCom[0] = "заблокировать";
            slovCom[1] = "окно";
            slovCom[2] = "в детской";
            break;
        case 20:
            slovCom[0] = "разблокировать";
            slovCom[1] = "окно";
            slovCom[2] = "в детской";
            break;

        case 21:
            slovCom[0] = "заблокировать";
            slovCom[1] = "окно";
            slovCom[2] = "в гостиной";
            break;
        case 22:
            slovCom[0] = "разблокировать";
            slovCom[1] = "окно";
            slovCom[2] = "в гостиной";
            break;

        case 23:
            slovCom[0] = "заблокировать";
            slovCom[1] = "дверь";
            slovCom[2] = "в спальне";
            break;
        case 24:
            slovCom[0] = "разблокировать";
            slovCom[1] = "дверь";
            slovCom[2] = "в спальне";
            break;

        case 25:
            slovCom[0] = "заблокировать";
            slovCom[1] = "дверь";
            slovCom[2] = "на балкон";
            break;
        case 26:
            slovCom[0] = "разблокировать";
            slovCom[1] = "дверь";
            slovCom[2] = "на балкон";
            break;

        case 27:
            slovCom[0] = "включить";
            slovCom[1] = "розетки";
            slovCom[2] = "на кухне";
            break;
        case 28:
            slovCom[0] = "отключить";
            slovCom[1] = "розетки";
            slovCom[2] = "на кухне";
            break;

        case 29:
            slovCom[0] = "включить";
            slovCom[1] = "розетки";
            slovCom[2] = "в гостиной";
            break;
        case 30:
            slovCom[0] = "отключить";
            slovCom[1] = "розетки";
            slovCom[2] = "в гостиной";
            break;

        case 31:
            slovCom[0] = "включить";
            slovCom[1] = "розетки";
            slovCom[2] = "в детской";
            break;
        case 32:
            slovCom[0] = "отключить";
            slovCom[1] = "розетки";
            slovCom[2] = "в детской";
            break;

        case 33:
            slovCom[0] = "включить";
            slovCom[1] = "телевизор";
            break;
        case 34:
            slovCom[0] = "выключить";
            slovCom[1] = "телевизор";
            break;

        default:
            cout << "Ошибка в определении команды\n";
             break;
    }

    // ВЫделение параметров и запись их в таблицу     // Список данных о словах для каждого пользователя - vector <userData> datatable;

    for (int i = 0; i < SLdata.size(); i++)       //Массив по словам
    {

        userData newU;
        newU.userId = usId;
        newU.slovo = slovCom[i];
        double kof[CofMFCC] = { 0 };

        /// Деление каждого слова на фреймы
        vector <vector <double>> FrameSL;
        int c = framesDivision(header, SLdata[i], FrameSL);         //количество отсчетов в фрейме


        // ПОЛУЧЕНИЕ MFCC КОЭФФИЦИЕНТОВ ДЛЯ СЛОВА для каждого фрейма?

        vector <vector <base>> x;

        for (int j = 0; j < FrameSL.size(); j++)              //проход по каждому фрейму
        {
            for (int k = 0; k < FrameSL[j].size(); k++)              //проход по всем значениям
            {
                x[j].push_back(complex <double>(FrameSL[j][k], 0.0));
            }

            FFT(x[j]);     //преобразование фурье

            getMFCCkoef(x, c, FrameSL.size(), kof);           //определение коэффициентов: входные данные, размерность, количество окон, вектор для записи коэф

        }

        cout << "Для пользователя с ID - " << newU.userId << " коэффициенты для слова " << newU.slovo << " = ";

        //Запись полученных зн в массив данных о словах
        for (int j = 0; j < CofMFCC; j++)
        {
            newU.slovData[j] = kof[j];
            cout << newU.slovData[j] << " ";
        }
        cout << endl;

        datatable.push_back(newU);

        // ДОБАВИТЬ !!!!! ТАКОЕ СЛОВО ДЛЯ ПОЛЬЗОВАТЕЛЯ УЖЕ ЕСТЬ
    }
    

    //.....???????????????......

    cout << "****** ДАННЫЕ УСПЕШЕНО ВНЕСЕНЫ *****\n";
}

void splittingWords(WAVHEADER header, vector <vector <double>> SLdata, double* indata)           //дробление по словам 
{
    const double minSlovo = 0.35;   //Минимальная длина слова sec
    const double minSil = 0.03;   // Минимальная длина тишины sec
    const double minHz = 31.622777;   // Минимальное значение тишины (15 dB)
    const double dsec = 1.0 / header.sampleRate;

    double currentSec = 0.0;
    int tempS = fabs(minSlovo * header.sampleRate) + 1;      //Минимальная длина слова в отсчетах
    int currTempS = 0;          //Отсчеты слов

    int tempT = fabs(minSil * header.sampleRate) + 1;      //Минимальная длина тишины в отсчетах
    int currTempT = 0;          //Отсчеты тишины

   // vector <vector <double>> SLdata;                        //Данные из записи разбитые по словам
    vector <double> sld;                                    //Данные одного слова
    // sld.push_back(-5.0);
    // SLdata.push_back(sld);

    bool writingS = false;
    int countS = 0;
    for (int i = 0; i < header.subchunk2Size; i++)      // цикл по данным //точно ++?
    {
        if (indata[i] > minHz)            //встретилась не тишина
        {
            currTempT = 0;

            if (sld.empty())            // не записано не одного значения слова
            {
                //начинается запись слова
               // SLdata.pop_back();          //удаление начального значения
               // sld.pop_back();
                writingS = true;
            }

            if (writingS == true)     //запись в пределах слова
            {
                sld.push_back(indata[i]);
                currTempS++;
            }
            else cout << "\nОШИБКА записи нового слова\n";         //новое слово

        }
        else if ((writingS == true) && ((currTempS < tempS) || (currTempT < tempT)))        //встретилась тишина, но между буквами. Условие : Записываем слово, длина слова меньше минимальной или пауза меньше минимальной между словами
        {
            sld.push_back(indata[i]);
            currTempS++;
            currTempT++;
        }
        else    //встретилась тишина между словами
        {
            if (writingS)           //Конец записывания слова
            {
                SLdata.push_back(sld);
                countS++;

                writingS = false;
                sld.clear();
                currTempS = 0;
            }
            currTempT++;
        }

    }
}
int framesDivision(WAVHEADER header, vector <double> slovoD, vector <vector <double>> FrameSL)       //деление слова на фреймы
{
    //Сколько отсчетов в фрейме
    int countPoint = (int)(sizeFrame / (1000.0 / header.sampleRate));           //*1000 т.к. работаем с мс 
    int coutFFT = 0;                                                            //количество фреймов для быстрого преобразования фурье

    //сколько отсчетов в перекрытии
    int stepPoint = (int)(stepFrame / (1000.0 / header.sampleRate));


    for (int i = 2; i < pow(2, 20) + 1; i *= 2)       //ближайщая степень двойки
    {
            if (countPoint <= i)
            {
                coutFFT = i;
                break;
            }
            cout << "*****ОШИБКА***** Число отсчетов больше " << pow(2, 20) << endl;
    }
       
    //vector <vector <double>> FrameSL;
    int k = 0;

    while (k < slovoD.size())
    {
        vector <double> frame;

        for (int i = 0; i < coutFFT; i++)           //один фрейм //Оконная функция для каждого фрагмента //если 0 ≤ t ≤ M-1   
        {
            if ((i < countPoint) && ((i + k) < slovoD.size()))           //заполняем данными произнесенного слова (после обработки оконной функцией) остаток нулями
                frame.push_back(windowsF(slovoD[i + k], i));                                                               //frame.push_back(slovoD[i + k]);
            else frame.push_back(0);

        }

        FrameSL.push_back(frame);
        k += stepPoint;
    }
    cout << "Слово разделилось на " << FrameSL.size() << " фрагментов\n";
     
    return coutFFT;
    

   
    


    

    


}


void initMatrix(int*** MatrCom) // Заполнение матрицы
{
        /* Количество всех возможных слов - 17. 0 - включить, 1 - отключить, 2 - выключить, 3 - заблокировать, 4 - разблокировать
    5 - свет, 6 - климат-контроль, 7 - розетки, 8 - телевизор, 9 - входную, 10 - дверь, 11 - окно, 12 - на кухне, 13 - в спальне, 14 - в гостиной, 15 - в детской, 16 - на балкон*/
    /* Количество возможных команд - 34: 1. Вкл свет в гостиной,    2. Выкл свет в гостиной,   3. Вкл свет в спальне,   4. Выкл свет в спальне, 5. Вкл свет на кухне,   6. Выкл свет на кухне,
    7. Вкл свет в детсткой, 8. Выкл свет в детской, 9. Вкл климат-контроль в гостиной, 10. Выкл климат-контроль в гостиной, 11. Вкл климат-контроль в спальне,  12. Выкл климат-контроль в спальне,
    13. Вкл климат-контроль на кухне,   14. Выкл климат-контроль на кухне,  15. Вкл климат-контроль в детской,  16. Выкл климат-контроль в детской, 17. Заблокировать входную дверь,    18. Разблокировать входную дверь,
    19. Заблокировать окно в детской,   20. Разблокировать окно в детской,  21. Заблокировать окно в гостиной, 22. Разблокировать окно в гостиной,  23. Заблокировать дверь в спальне,  24. Разблокировать дверь в спальне,
    25. Заблокировать дверь на балкон,  26. Разблокировать дверь на балкон, 27. Включить розетки на кухне, 28. Отключить розетки на кухне,  29. Включить розетки в гостиной,    30. Отключить розетки в гостиной,
    31. Включить розетки в детской, 32. Отключить розетки в детской,    33. Вкл телевизор,  34. Выкл телевизор */
    
    // ЗАПОЛНЕНИЕ МАТРИЦЫ

    //0. Следующее слово после "Включить"
            //0 - включить. Команды, коответствующие данному переходу, отсутствуют
            //1 - отключить. Команды, коответствующие данному переходу, отсутствуют
            //2 - выключить. Команды, коответствующие данному переходу, отсутствуют
            //3 - заблокировать. Команды, коответствующие данному переходу, отсутствуют
            //4 - разблокировать. Команды, коответствующие данному переходу, отсутствуют
    //5 - свет. Команды: 1,3,5,7                       
    MatrCom[0][5][0] = 1;
    MatrCom[0][5][1] = 3;
    MatrCom[0][5][2] = 5;
    MatrCom[0][5][4] = 7;
    //6 - климат-контроль. Команды: 9,11,13,15
    MatrCom[0][6][0] = 9;
    MatrCom[0][6][1] = 11;
    MatrCom[0][6][2] = 13;
    MatrCom[0][6][4] = 15;
    //7 - розетки. Команды: 27,29,31
    MatrCom[0][7][0] = 27;
    MatrCom[0][7][1] = 29;
    MatrCom[0][7][2] = 31;

    //8 - телевизор. Команда: 33
    MatrCom[0][8][0] = 33;
    //9 - входную. Команды, коответствующие данному переходу, отсутствуют
    //10 - дверь. Команды, коответствующие данному переходу, отсутствуют
    //11 - окно. Команды, коответствующие данному переходу, отсутствуют
//12 - на кухне. Команды: 5,13,27
    MatrCom[0][12][0] = 5;
    MatrCom[0][12][1] = 13;
    MatrCom[0][12][2] = 27;

    //13 - в спальне. Команды: 3,11
    MatrCom[0][13][0] = 3;
    MatrCom[0][13][1] = 11;

    //14 - в гостиной. Команды: 1,9,29
    MatrCom[0][14][0] = 1;
    MatrCom[0][14][1] = 9;
    MatrCom[0][14][2] = 29;

    //15 - в детской. Команды: 7,15,31
    MatrCom[0][15][0] = 7;
    MatrCom[0][15][1] = 15;
    MatrCom[0][15][2] = 31;
    //16 - на балкон. Команды, коответствующие данному переходу, отсутствуют

//__________________________________________________________________________________________
//1. Следующее слово после "Отключить"

//7 - розетки. Команды: 28,30,32
    MatrCom[1][7][0] = 28;
    MatrCom[1][7][1] = 30;
    MatrCom[1][7][2] = 32;
    //12 - на кухне. Команда 28
    MatrCom[1][12][0] = 28;
    //14 - в гостиной. Команда 30
    MatrCom[1][14][0] = 30;
    //15 - в детской. Команда 32
    MatrCom[1][15][0] = 32;

    //__________________________________________________________________________________________
    //2. Следующее слово после "Выключить"

    //5 - свет. Команды: 2,4,6,8
    MatrCom[2][5][0] = 2;
    MatrCom[2][5][1] = 4;
    MatrCom[2][5][2] = 6;
    MatrCom[2][5][4] = 8;
    //6 - климат-контроль. Команды: 10,12,14,16
    MatrCom[2][6][0] = 10;
    MatrCom[2][6][1] = 12;
    MatrCom[2][6][2] = 14;
    MatrCom[2][6][4] = 16;
    //8 - телевизор. Команда 34
    MatrCom[2][8][0] = 34;
    //12 - на кухне. Команды: 6,14
    MatrCom[2][12][0] = 6;
    MatrCom[2][12][1] = 14;
    //13 - в спальне. Команды: 4,12
    MatrCom[2][13][0] = 4;
    MatrCom[2][13][1] = 12;
    //14 - в гостиной. Команды: 2,10
    MatrCom[2][14][0] = 2;
    MatrCom[2][14][1] = 10;
    //15 - в детской. Команды: 8,16
    MatrCom[2][15][0] = 8;
    MatrCom[2][15][1] = 16;

    //__________________________________________________________________________________________
    //3. Следующее слово после "Заблокировать"

    //9 - входную. Команда 17
    MatrCom[3][9][0] = 17;
    //10 - дверь. Команды: 17,23,25
    MatrCom[3][10][0] = 17;
    MatrCom[3][10][1] = 23;
    MatrCom[3][10][2] = 25;
    //11 - окно. Команды: 19,21
    MatrCom[3][11][0] = 19;
    MatrCom[3][11][1] = 21;
    //13 - в спальне. Команда 23
    MatrCom[3][13][0] = 23;
    //14 - в гостиной. Команда 21
    MatrCom[3][14][0] = 21;
    //15 - в детской. Команда 19
    MatrCom[3][15][0] = 19;
    //16 - на балкон. Команда 25
    MatrCom[3][16][0] = 25;

    //__________________________________________________________________________________________
    //4. Следующее слово после "Разблокировать"

    //9 - входную. Команда 18
    MatrCom[4][9][0] = 18;
    //10 - дверь. Команды: 18,24,26
    MatrCom[4][10][0] = 18;
    MatrCom[4][10][1] = 24;
    MatrCom[4][10][2] = 26;
    //11 - окно. Команды: 20,22
    MatrCom[4][11][0] = 20;
    MatrCom[4][11][1] = 22;
    //13 - в спальне. Команда 24
    MatrCom[4][13][0] = 24;
    //14 - в гостиной. Команда 22
    MatrCom[4][14][0] = 22;
    //15 - в детской. Команда 20
    MatrCom[4][15][0] = 20;
    //16 - на балкон. Команда 26
    MatrCom[4][16][0] = 26;

    //__________________________________________________________________________________________
    //5. Следующее слово после "Свет"

    //0 - включить. Команды: 1,3,5,7
    MatrCom[5][0][0] = 1;
    MatrCom[5][0][1] = 3;
    MatrCom[5][0][2] = 5;
    MatrCom[5][0][4] = 7;
    //2 - выключить. Команды: 2,4,6,8
    MatrCom[5][2][0] = 2;
    MatrCom[5][2][1] = 4;
    MatrCom[5][2][2] = 6;
    MatrCom[5][2][4] = 8;
    //12 - на кухне. Команды: 5,6
    MatrCom[5][12][0] = 5;
    MatrCom[5][12][1] = 6;
    //13 - в спальне. Команды: 3,4
    MatrCom[5][13][0] = 3;
    MatrCom[5][13][1] = 4;
    //14 - в гостиной. Команды: 1,2
    MatrCom[5][14][0] = 1;
    MatrCom[5][14][1] = 2;
    //15 - в детской. Команды: 7,8
    MatrCom[5][15][0] = 7;
    MatrCom[5][15][1] = 8;

    //__________________________________________________________________________________________
    //6. Следующее слово после "Климат-контроль"

    //0 - включить. Команды: 9,11,13,15
    MatrCom[6][0][0] = 9;
    MatrCom[6][0][1] = 11;
    MatrCom[6][0][2] = 13;
    MatrCom[6][0][4] = 15;
    //2 - выключить. Команды: 10,12,14,16
    MatrCom[6][2][0] = 10;
    MatrCom[6][2][1] = 12;
    MatrCom[6][2][2] = 14;
    MatrCom[6][2][4] = 16;
    //12 - на кухне. Команды: 13,14
    MatrCom[6][12][0] = 13;
    MatrCom[6][12][1] = 14;
    //13 - в спальне. Команды: 11,12
    MatrCom[6][13][0] = 11;
    MatrCom[6][13][1] = 12;
    //14 - в гостиной. Команды: 9,10
    MatrCom[6][14][0] = 9;
    MatrCom[6][14][1] = 10;
    //15 - в детской. Команды: 14,16
    MatrCom[6][15][0] = 14;
    MatrCom[6][15][1] = 16;

    //__________________________________________________________________________________________
    //7. Следующее слово после "Розетки"

    //0 - включить. Команды: 27,29,31
    MatrCom[7][0][0] = 27;
    MatrCom[7][0][1] = 29;
    MatrCom[7][0][2] = 31;
    //1 - отключить. Команды: 28,30,32
    MatrCom[7][1][0] = 28;
    MatrCom[7][1][1] = 30;
    MatrCom[7][1][2] = 32;
    //12 - на кухне. Команды: 27,28
    MatrCom[7][12][0] = 27;
    MatrCom[7][12][1] = 28;
    //14 - в гостиной. Команды: 29,30
    MatrCom[7][14][0] = 29;
    MatrCom[7][14][1] = 30;
    //15 - в детской. Команды: 31,32
    MatrCom[7][15][0] = 31;
    MatrCom[7][15][1] = 32;

    //__________________________________________________________________________________________
    //8. Следующее слово после "телевизор"

    //0 - включить. Команда 33
    MatrCom[8][0][0] = 33;
    //2 - выключить. Команда 34
    MatrCom[8][2][0] = 34;

    //__________________________________________________________________________________________
    //9. Следующее слово после "входную"

    //3 - заблокировать. Команда 17
    MatrCom[9][3][0] = 17;
    //4 - разблокировать. Команда 18
    MatrCom[9][4][0] = 18;
    //10 - дверь. Команды: 17,18
    MatrCom[9][10][0] = 17;
    MatrCom[9][10][1] = 18;

    //__________________________________________________________________________________________
    //10. Следующее слово после "дверь"

    //3 - заблокировать. Команды: 17,23,25
    MatrCom[10][3][0] = 17;
    MatrCom[10][3][1] = 23;
    MatrCom[10][3][2] = 25;
    //4 - разблокировать. Команды: 18,24,26
    MatrCom[10][4][0] = 18;
    MatrCom[10][4][1] = 24;
    MatrCom[10][4][2] = 26;
    //9 - входную. Команды: 17,18
    MatrCom[10][9][0] = 17;
    MatrCom[10][9][1] = 18;
    //13 - в спальне. Команды: 23,24
    MatrCom[10][13][0] = 23;
    MatrCom[10][13][1] = 24;
    //16 - на балкон. Команды: 25,26
    MatrCom[10][16][0] = 25;
    MatrCom[10][16][1] = 26;

    //__________________________________________________________________________________________
    //11. Следующее слово после "Окно"

    //3 - заблокировать. Команды: 19,21
    MatrCom[11][3][0] = 19;
    MatrCom[11][3][1] = 21;
    //4 - разблокировать. Команды: 20,22
    MatrCom[11][4][0] = 20;
    MatrCom[11][4][1] = 22;
    //14 - в гостиной. Команды: 21,22
    MatrCom[11][14][0] = 21;
    MatrCom[11][14][1] = 22;
    //15 - в детской. Команды: 19,20
    MatrCom[11][15][0] = 19;
    MatrCom[11][15][1] = 20;

    //__________________________________________________________________________________________
    //12. Следующее слово после "на кухне"

    //0 - включить. Команды: 5,13,27
    MatrCom[12][0][0] = 5;
    MatrCom[12][0][1] = 13;
    MatrCom[12][0][2] = 27;
    //1 - отключить. Команда 28
    MatrCom[12][1][0] = 28;
    //2 - выключить. Команды: 6,14
    MatrCom[12][2][0] = 6;
    MatrCom[12][2][1] = 14;
    //5 - свет. Команды: 5,6
    MatrCom[12][5][0] = 5;
    MatrCom[12][5][1] = 6;
    //6 - климат-контроль. Команды: 13,14
    MatrCom[12][6][0] = 13;
    MatrCom[12][6][1] = 14;
    //7 - розетки. Команды: 27,28
    MatrCom[12][7][0] = 27;
    MatrCom[12][7][1] = 28;

    //__________________________________________________________________________________________
    //13. Следующее слово после "В спальне"

    //0 - включить. Команды: 3,11
    MatrCom[13][0][0] = 3;
    MatrCom[13][0][1] = 11;
    //2 - выключить. Команды: 4,12
    MatrCom[13][2][0] = 3;
    MatrCom[13][2][1] = 11;
    //3 - заблокировать. Команда 23
    MatrCom[13][3][0] = 23;
    //4 - разблокировать. Команда 24
    MatrCom[13][4][0] = 24;
    //5 - свет. Команды: 3,4
    MatrCom[13][5][0] = 3;
    MatrCom[13][5][1] = 4;
    //6 - климат-контроль. Команды: 11,12
    MatrCom[13][6][0] = 11;
    MatrCom[13][6][1] = 12;
    //10 - дверь. Команды: 23,24
    MatrCom[13][10][0] = 23;
    MatrCom[13][10][1] = 24;

    //__________________________________________________________________________________________
    //14. Следующее слово после "в гостиной"

    //0 - включить. Команды: 1,9,29
    MatrCom[14][0][0] = 1;
    MatrCom[14][0][1] = 9;
    MatrCom[14][0][2] = 29;
    //1 - отключить. Команда 30
    MatrCom[14][1][0] = 30;
    //2 - выключить. Команды: 2,10
    MatrCom[14][2][0] = 2;
    MatrCom[14][2][1] = 10;
    //3 - заблокировать. Команда 21
    MatrCom[14][3][0] = 21;
    //4 - разблокировать. Команда 22
    MatrCom[14][4][0] = 22;
    //5 - свет. Команды: 1,2
    MatrCom[14][5][0] = 1;
    MatrCom[14][5][1] = 2;
    //6 - климат-контроль. Команды: 9,10
    MatrCom[14][6][0] = 9;
    MatrCom[14][6][1] = 10;
    //7 - розетки. Команды: 29,30
    MatrCom[14][7][0] = 29;
    MatrCom[14][7][1] = 30;
    //11 - окно. Команды: 21,22
    MatrCom[14][11][0] = 21;
    MatrCom[14][11][1] = 22;

    //__________________________________________________________________________________________
    //15. Следующее слово после "в детской"

    //0 - включить. Команды: 7,15,31
    MatrCom[15][0][0] = 7;
    MatrCom[15][0][1] = 15;
    MatrCom[15][0][2] = 31;
    //1 - отключить. Команда 32
    MatrCom[15][1][0] = 32;
    //2 - выключить. Команды: 8,16
    MatrCom[15][2][0] = 8;
    MatrCom[15][2][1] = 16;
    //3 - заблокировать. Команда 19
    MatrCom[15][3][0] = 19;
    //4 - разблокировать. Команда 20
    MatrCom[15][4][0] = 20;
    //5 - свет. Команды: 7,8
    MatrCom[15][5][0] = 7;
    MatrCom[15][5][1] = 8;
    //6 - климат-контроль. Команды: 15,16
    MatrCom[15][6][0] = 15;
    MatrCom[15][6][1] = 16;
    //7 - розетки. Команды: 31,32
    MatrCom[15][7][0] = 31;
    MatrCom[15][7][1] = 32;
    //11 - окно. Команды: 19,20
    MatrCom[15][11][0] = 19;
    MatrCom[15][11][1] = 20;

    //__________________________________________________________________________________________
    //16. Следующее слово после "на балкон"

    //3 - заблокировать. Команда 25
    MatrCom[16][3][0] = 25;
    //4 - разблокировать. Команда 26
    MatrCom[16][4][0] = 26;
    //10 - дверь. Команды: 25,26
    MatrCom[16][10][0] = 25;
    MatrCom[16][10][1] = 26;
}

int classification(double kof[], vector <userData> datatable)
{
    // y = b0 + b1 * x1 + b2* x2 ....
    vector<double>error;                // error values
    vector <double> currentMFCC;

    // Будем классифицировать путем уменьшения размерности пространства 
    // Относительно каждого коэффициента строится плоскость
    int countMFCC = 0;
    bool end = false;
    int res = -1;

    //начальные значения группы
    vector <int> NinGroup;

    for (int i = 0; i < datatable.size(); i++)          //номера в группе/ пЕРВОНАЧАЛЬНО - 1 КЛАСС  
    {
        NinGroup.push_back(i);
    }

    while (!end)
    {
        for (int i = 0; i < NinGroup.size(); i++)          //Проход оп всем словам 
        {
            
            currentMFCC.push_back(datatable[NinGroup[i]].slovData[countMFCC]);

        }
        NinGroup.clear();


        // поиск самого большого расстояния между соседними значениями
        double maxWay = 0.0;
        double currWay;
        int border = 0;

        sort(currentMFCC.begin(), currentMFCC.end());

        for (int i = 0; i < currentMFCC.size() - 1; i++)
        {
            currWay = fabs(currentMFCC[i + 1] - currentMFCC[i]);
            if (currWay >= maxWay)
            {
                maxWay = currWay;
                border = i;
            }

        }

        //ПЛОСКОСТЬ РАЗДЕЛЯЮЩАЯ 2 КЛАССА
        
        

        //в какой группе  искомая точка?
        int group;
        if (kof[countMFCC] <= currentMFCC[border])
            group = 0;
        else group = 1;

        for (int i = 0; i < currentMFCC.size(); i++)
        {
            if (datatable[i].slovData[countMFCC] <= currentMFCC[border])                //относится к 0 группе
            {
                if (group == 0)
                    NinGroup.push_back(i);
            }
            else if (group == 1)
                NinGroup.push_back(i);

        }

        cout << "В группе " << NinGroup.size() << "значений\n";
        countMFCC++;

        if (NinGroup.size() == 1)              //определили однозначно
        {
            end = true;
            res = NinGroup[0];
        }
            

        /*
        for (int i = 0; i < currentMFCC.size(); i++)
        {
            if (i <= border)
                y.push_back(0);
            else y.push_back(1);
        } */
    }
   
    if (res > -1)
        return res;
    else cout << "********* ОШИБКА КЛАССИФИКАТОРА **************\n";

    return -1;

 
}
void phraseCommands(char defC[], int*** MatrCom, int last, int out)           //Нарощение команды. Определение к какой команде относится переход
{
    if (MatrCom[last][out][0] == 0)
    {
        cout << "ОШИБКА!!! Переход " << last << " -> " << out << " не присутствует ни в каких командах\n";
        return;
    }

    //char* step = new char[DIM3];
    cout << " Переход " << last << " -> " << out << " присутствует в командах: ";
    for (int i = 0; i < DIM3; i++)
    {
        int c = MatrCom[last][out][i];
        cout << c << " ";
        defC[c] += 1;
    }   
    cout << endl;
 
   // delete[] step;
}
