#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// Prepared By Eren Durucan

// DESCRIPTION: This program simulates the DINING-PHILOSOPHERS problem with N tea-drinking philosophers.
//   COMPILE : gcc -o df df.c -lpthread
// PARAMETER : <philosopherCount> -> The only parameter the user needs to enter is the number of philosophers.
//       RUN : ./df 6

// Durum numaralari
#define THINKING 0
#define THIRSTY 1
#define POURING 2
#define DRINKING 3

// Ekrana cikti yazdirma fonksiyonu icin gereken sabit
#define HIZALAMA_SABITI 3

// Degiskenler
char *durumlar[4] = {"THINKING  (", "THIRSTY   (", "POURING   (", "DRINKING  ("};
int *durum;         // Her filozofun durumunu tutar.

int filozofSayisi;  // Kullanici tarafindan girilen filozof sayisi
int *filozofNumaralari; // Her filozofun numarasini tutar.
int eklenenCay;     // Caydanliga eklenen cay sayisini tutar.
int kalanCay;       // Caydanlikta kalan cay sayisini tutar.
int *icilenCay;     // Her filozofun icilen cay sayisini tutar.

int yazdirmaImlec = 0; // print_status fonksiyonunun hizalanmasi icin kullanilir.

// Semaforlar
sem_t cay;           // Cay bardagi icin semafor
sem_t caydanlik;     // Caydanlik icin semafor
sem_t sekerlik;      // Her filozofun sekerlik icin semaforu
sem_t *cayTabagi;    // Her filozofun cay tabagi icin semaforu
sem_t yazdirmaMutex; // print_status fonksiyonu cagirilirken yazdirma isleminin ayni anda yapilmasini engellemek icin

// Kontrol mekanizmasi icin gerekli fonksiyonlar
void *filozofKontrol(void *args);
void *garsonKontrol();
void *print_status();
void kontrolVeYazdir(int filozofNumara);
void semaforlariBaslat();
void threadleriBaslat(pthread_t *filozoflar, pthread_t *garson);
void kaynaklariTemizle();

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        // printf("Kullanim: ./df <filozofSayisi>\n");
        return 1;
    }

    // Kullanici arguman olarak filozof sayisini girmelidir.
    filozofSayisi = atoi(argv[1]);

    // Filozof sayisi alindiktan sonra caydanliga eklenen cay sayisi hesaplanir.
    eklenenCay = 2 * filozofSayisi;

    // Random fonksiyonu icin seed ayarla. (Garsonun cay eklemesi icin)
    srand(time(NULL));

    // Her filozofun durumunu ve icilen cay sayisini tutacak dizileri olustur.
    durum = (int *)malloc(sizeof(int) * filozofSayisi);
    icilenCay = (int *)malloc(sizeof(int) * filozofSayisi);

    // Baslangicta her filozof dusunuyor ve hic cay icmemis.
    for (int i = 0; i < filozofSayisi; i++)
    {
        durum[i] = THINKING;
        icilenCay[i] = 0;
    }

    semaforlariBaslat();

    // Her filozofun numarasini tutacak dizi olustur. (Threadler icin)
    filozofNumaralari = (int *)malloc(sizeof(int) * filozofSayisi);
    for (int i = 0; i < filozofSayisi; i++) {
        filozofNumaralari[i] = i;
    }
    
    // Her filozof ve garson icin thread belirle.
    pthread_t filozoflar[filozofSayisi], garson;
    threadleriBaslat(filozoflar, &garson);

    // Garson threadini isleme al.
    if (pthread_join(garson, NULL) != 0)
    {
        return 1;
    }

    // Her filozof threadini isleme al.
    for (int i = 0; i < filozofSayisi; i++)
    {
        if (pthread_join(filozoflar[i], NULL) != 0)
        {
            return 1;
        }
    }

    kaynaklariTemizle();
    return 0;
}

// Her filozofun durumunu ve cay sayisini yazdir.
void *print_status()
{
    for (int imlecNo = 0; imlecNo < yazdirmaImlec; imlecNo++)
    {
        printf("\033[F");
    }
    yazdirmaImlec = filozofSayisi + 3;

    printf("Eklenen cay %d     \n", eklenenCay);
    for (int fNo = 0; fNo < filozofSayisi; fNo++)
    {
        printf("P%d: %s%d)    \n", fNo + 1, durumlar[durum[fNo]], icilenCay[fNo]);
    }
    printf("kalan cay: %d     \n", kalanCay);
    printf("-----\n");
}

void kontrolVeYazdir(int filozofNumara) {
    // Filozofun cay bardagi ve cay tabagi olup olmadigini kontrol et.
    int bardakVeTabak = (filozofNumara % 2 != ((filozofNumara + 1) % filozofSayisi) % 2);
    if (!bardakVeTabak) {
        return; // Filozofun cay bardagi ve cay tabagi yoksa kontrol etme.
    }

    sem_wait(&yazdirmaMutex);   // Yazdirma islemi baslamadan once mutex kilidini al.

    print_status();             // Durumlari yazdir.

    // Ilk yazdirma islemi tamamlandiktan sonra imleci duzgun konuma getir.
    yazdirmaImlec = filozofSayisi + HIZALAMA_SABITI;

    sem_post(&yazdirmaMutex);   // Yazdirma islemi bittikten sonra mutex kilidini serbest birak.
}

// Her filozofun durumunu kontrol et.
void *filozofKontrol(void *filozofParam)
{
    // Filozof numarasini al.
    int filozofNumara = filozofNumaralari[*(int *)filozofParam];
    // Filozofun durumunu al.
    int *filozofDurum = &durum[filozofNumara];

    // Surekli calisan dongu kontrolu.
    while (1)
    {
        switch (*filozofDurum) {
            case THINKING:
                sleep(1);                           // Dusunme suresi

                *filozofDurum = THIRSTY;            // Susama durumuna gec.

                break;

            case THIRSTY:
                // Filozofun cay bardagi ve cay tabagi olup olmadigini kontrol et.
                if (filozofNumara % 2 != ((filozofNumara + 1) % filozofSayisi) % 2) {
                    sem_wait(&cayTabagi[(filozofNumara + 1) % filozofSayisi]);  // Komsu cay tabagina erisim icin semaforu kilitle.
                    sem_wait(&cayTabagi[filozofNumara]);                        // Cay tabagina erisim icin semaforu kilitle.

                    if (filozofNumara % 2 == 1) {   // Tek sayili filozoflar sekerligi alir.
                        sem_wait(&sekerlik);
                    }
                    
                    *filozofDurum = POURING;        // Cay doldurma durumuna gec.
                }

                break;

            case POURING:
                sem_wait(&caydanlik);               // Caydanliga erisim icin semaforu kilitle.
                sem_wait(&cay);                     // Cay bardagina erisim icin semaforu kilitle.
                sem_post(&caydanlik);               // Caydanliga erisim icin semaforu serbest birak.

                *filozofDurum = DRINKING;           // Icme durumuna gec.

                break;

            case DRINKING:
                sleep(1);                           // Cay icme suresi

                sem_post(&cayTabagi[(filozofNumara + 1) % filozofSayisi]);  // Komsu cay tabagini serbest birak.
                sem_post(&cayTabagi[filozofNumara]);                        // Kendi cay tabagini serbest birak.

                if (filozofNumara % 2 == 1) {       // Tek sayili filozoflar sekerligi birakir.
                    sem_post(&sekerlik);
                }

                icilenCay[filozofNumara]++;         // Cay sayisini arttir.

                *filozofDurum = THINKING;           // Dusunmeye basla.

                break;

            default:                                // Hata durumu
                break;
        }

        kontrolVeYazdir(filozofNumara);             // Her degisimde durumlari kontrol et ve yazdir.
    }
}

// Garsonun cay eklemesini kontrol et.
void *garsonKontrol()
{
    while (1)
    {
        // Anlik Cay miktarini al.
        sem_getvalue(&cay, &kalanCay);
        
        sem_wait(&caydanlik);           // Caydanliga erisim icin semaforu kilitle.
        
        // Cay kalmadiysa garson yeni cay eklesin.
        if (kalanCay == 0)
        {
            sleep(5);                       // Her tur gecisinde 5 saniye bekle.

            eklenenCay = (rand() % (5 * filozofSayisi)) + 1; // 1 ile 5N arasinda rasgele sayida cay ekle.

            for (int i = 0; i < filozofSayisi; i++)
            {
                icilenCay[i] = 0;           // Her filozofun icilen cay sayisini sifirla.
            }

            sem_init(&cay, 0, eklenenCay);  // Caydanlik semaforunu baslat.
        }

        sem_post(&caydanlik);           // Caydanliga erisim icin semaforu serbest birak.
    }
}

// Cay bardagi, cay tabagi, sekerlik, caydanlik ve yazdirma islemleri icin semaforlari baslat.
void semaforlariBaslat() {
    sem_init(&caydanlik, 0, 1);
    sem_init(&sekerlik, 0, 1);
    sem_init(&cay, 0, eklenenCay);
    sem_init(&yazdirmaMutex, 0, 1);

    cayTabagi = (sem_t *)malloc(sizeof(sem_t) * filozofSayisi);
    for (int i = 0; i < filozofSayisi; i++) {
        sem_init(&cayTabagi[i], 0, 1);
    }
}

// Her filozof ve garson icin threadleri baslat.
void threadleriBaslat(pthread_t *filozoflar, pthread_t *garson) {
    // Garson icin thread olustur.
    if (pthread_create(garson, NULL, &garsonKontrol, NULL) != 0) {
        exit(1);
    }

    // Her filozof icin thread olustur. 
    // FilozofNo degiskeni, filozofun kacinci sirada oldugunu tutar.
    // Ilk parametre        : Thread adresi
    // Ikinci parametre     : Thread ozellikleri
    // Ucuncu parametre     : Thread'in calistiracagi fonksiyon
    // Dorduncu parametre   : Fonksiyona gonderilecek parametre
    for (int i = 0; i < filozofSayisi; i++) {
        if (pthread_create(&filozoflar[i], NULL, &filozofKontrol, &i) != 0) {
            exit(1);
        }
    }
}

// Tum semaforlari ve dizileri serbest birak.
void kaynaklariTemizle() {
    sem_destroy(&cay);
    sem_destroy(&caydanlik);
    sem_destroy(&sekerlik);
    sem_destroy(&yazdirmaMutex);
    for (int i = 0; i < filozofSayisi; i++) { sem_destroy(&cayTabagi[i]); }

    free(durum);
    free(icilenCay);
    free(cayTabagi);
}