/*******************************************************************************
      Algoritmo de envio de e-mail com anexo de arquivos da SPIFFS com ESP32
                                Sketch de Exemplo

                            Criado em 01 abr. 2022
                               por Michel Galvão

  Eletrogate - Loja de Arduino \\ Robótica \\ Automação \\ Apostilas \\ Kits
                            https://www.eletrogate.com/
*******************************************************************************/

// Inclusão da(s) biblioteca(s)
#include <WiFi.h> // Biblioteca nativa do ESP32
#include <ESP_Mail_Client.h>  // Biblioteca de cliente de e-mail do 
//                                Arduino para enviar.
//                                Referência: https://github.com/mobizt/ESP-Mail-Client

// Configurações da rede WiFi à se conectar
#define WIFI_SSID "<O_SSID_de_sua_rede_WiFi>"
#define WIFI_PASSWORD "<A_senha_de_sua_rede_WiFi>"

// Configurações do SMTP host
#define SMTP_HOST "smtp.gmail.com" // SMTP host
#define SMTP_PORT 465 // SMTP port

// As credenciais de login
#define AUTOR_EMAIL "<O_email_remetente>"
#define AUTOR_SENHA "<A_senha_de_app_do_email_remetente>"

// O objeto de sessão SMTP usado para envio de e-mail
SMTPSession smtp;

// Protótipos das Funções
void smtpCallback(SMTP_Status status);

void setup() {
  // Inicia Serial
  Serial.begin(115200);
  Serial.println();
  delay(1000);

  // Inicia conexão WiFi
  Serial.println("Conectando à rede WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) // Enquanto o status de conexão ao WiFi
    //                                      não for bem sucedido, ...
  {
    Serial.print(".");
    delay(200);
  }

  // Mostra na Serial que a conexão está realoizado e mostra o IP do ESP32 na rede
  Serial.println("\nWiFi conectado.");

  // Habilita a depuração via porta serial:
  //    0: nenhuma depuração
  //    1: deburação básica
  smtp.debug(0);

  // Define a função de retorno de chamada para obter os resultados de envio
  smtp.callback(smtpCallback);

  //Define a mensagem de texto à ser enviada
  String messageTXT = "Teste de envio de E-mail com anexos.";
  // função que envia o e-mail

  // Objeto para declarar os dados de configuração da sessão
  ESP_Mail_Session session;
  // Defina os dados de configuração da sessão
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTOR_EMAIL;
  session.login.password = AUTOR_SENHA;
  session.login.user_domain = "";
  // Defina o tempo de configuração do NTP
  session.time.ntp_server = F("time.google.com"); // Utilizado o NTP do Google:
  //                                                  https://developers.google.com/time
  session.time.gmt_offset = -3; // define o deslocamento em segundos
  //                                do fuso horário local em relação ao
  //                                GMT do Meridiano de Greenwich.

  session.time.day_light_offset = 0; // define o deslocamento em segundos do
  //                                      fuso horário local. Este valor costuma
  //                                      ser 3600 para horário de verão +1h ou
  //                                      0 para fusos sem horário de verão.

  // Instanciação do objeto da classe de mensagem
  SMTP_Message message;

  // Definição os cabeçalhos das mensagens
  message.sender.name = "Michel Galvão";
  message.sender.email = AUTOR_EMAIL;
  message.subject = "Teste de E-mail com Anexo";
  message.addRecipient("Michel Galvão", "<O_email_destinatario>");
  message.text.content = messageTXT.c_str();

  // O conjunto de caracteres de mensagem de texto html, por exemplo:
  //  us-ascii
  //  utf-8
  //  utf-7
  // O valor padrão é utf-8
  message.text.charSet = "utf-8";

  // A codificação de transferência de conteúdo. Ex:
  //  enc_7bit ou "7bit" (não codificado)
  //  enc_qp ou "quoted-printable" (codificado)
  //  enc_base64 ou "base64" (codificado)
  //  enc_binary ou "binary" (não codificado)
  //  enc_8bit ou "8bit" (não codificado)
  //  O valor padrão é "7bit"
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  // A prioridade da mensagem:
  //  esp_mail_smtp_priority_high ou 1
  //  esp_mail_smtp_priority_normal ou 3
  //  esp_mail_smtp_priority_low ou 5
  //  O valor padrão é esp_mail_smtp_priority_low
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  // As notificações de status de entrega, Ex:
  //  esp_mail_smtp_notify_never
  //  esp_mail_smtp_notify_success
  //  esp_mail_smtp_notify_failure
  //  esp_mail_smtp_notify_delay
  //  O valor padrão é esp_mail_smtp_notify_never
  message.response.notify = esp_mail_smtp_notify_success |
                            esp_mail_smtp_notify_failure |
                            esp_mail_smtp_notify_delay;

  // O item de dados do anexo
  SMTP_Attachment att;
  
  // Defina as informações do anexo, por exemplo: nome do arquivo, tipo MIME, 
  //  caminho do arquivo, tipo de armazenamento do arquivo, codificação de 
  //  transferência e codificação de conteúdo.
  att.descr.filename = "exemplo de imagem.png";
  att.descr.mime = "image/png"; //binary data
  att.file.path = "/exemplo de imagem.png";
  att.file.storage_type = esp_mail_file_storage_type_flash;
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  // Adiciona anexo à mensagem
  message.addAttachment(att);

  message.resetAttachItem(att); //  limpa os dados internos do item de anexo
  //                                  para estar pronto para reutilização.
  att.descr.filename = "arquivo de exemplo.txt"; // nome do arquivo
  att.descr.mime = "text/plain"; // tipo do arquivo
  att.file.path = "/arquivo de exemplo.txt"; // o caminho do arquivo
  att.file.storage_type = esp_mail_file_storage_type_flash; // O tipo de armazenamento
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64; //A opção de
  //                                codificar o conteúdo para transferência de dados.

  // Adiciona anexo à mensagem
  message.addAttachment(att);

  // Conecte-se ao servidor com a configuração da sessão
  if (!smtp.connect(&session))
    return;

  // Começa a enviar e-mail e fecha a sessão
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Erro ao enviar e-mail, " + smtp.errorReason());
  }

  while (1); // loop infinito

}

void loop() {}

/**
    Função de retorno de chamada para obter o status de envio de e-mail
*/
void smtpCallback(SMTP_Status status) {
  // Imprime o status atual
  Serial.println(status.info());

  // Imprima o resultado do envio
  if (status.success()) {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Mensagem enviada com sucesso: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Falha na mensagem enviada: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Obter o item de resultado */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "sucesso" : "fracassado");
      ESP_MAIL_PRINTF("Data/Hora: %d/%d/%d %d:%d:%d\n",
                      dt.tm_year + 1900,
                      dt.tm_mon + 1,
                      dt.tm_mday,
                      dt.tm_hour,
                      dt.tm_min,
                      dt.tm_sec);
      ESP_MAIL_PRINTF("Recebedor: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Sujeito: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
