//Код с функцией приёма и обработки пакета данных из тестового задания.

//uint8_t Rx_Function(uint8_t source, uint8_t receiver, uint8_t *data) Функция обработки принятого пакета
//Возвращает 0 приудачной обработки всего принятого пакета.
// 1 если CRC-8 не совпала.
// 2 если адрес отправителя не совпал.
// 3 если адрес приёмника не совпал.
//Параметры функции "uint8_t source" адрес передатчика.
// "uint8_t receiver" адрес приёмника.
// "uint8_t *data" массив данных. 

uint8_t buff[12], tmp_buf[1];
uint8_t  trig=0, cout_rx=0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)  //Выполняется если пакет не полный
{
        if(htim->Instance == TIM11)
        {
        	HAL_TIM_Base_Stop_IT(&htim11);
        					   TIM11->CNT = 0;
        					   cout_rx = 0;
        					   HAL_UART_Receive_IT(&huart1, tmp_buf, 1);
        					   GPIOD->ODR = 4;
        }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)    //Перехот по вектору прерывания когда принят один байт USART
{
          if(huart == &huart1)
          {
        	  if(!cout_rx)                                 //Блокировать дальнейший вызов пуска таймера 11
        	  HAL_TIM_Base_Start_IT(&htim11);              //Запуск таймера 11 нужен для контроля вхождения полного пакета
        	  trig=1;
          }
}

uint8_t Rx_Function(uint8_t source, uint8_t receiver, uint8_t *data)  //Функция обработки принятого пакета
{
	uint8_t crc = 0;
	uint8_t count=0;
	uint8_t count_crc;

	union                                                           //Союз для обединения типов части данных из пакета в
	                                                                //Переменную uint16_t port_rx
	{
		uint8_t rx_data[4];
		uint16_t port_rx;
	}port_rx;

	count_crc = data[4]+6;                                          //Принемаем байт длины пакета без одного байта CRC-8
	while (count_crc--)                                             //Цикл вычисления CRC-8
	    {
	        crc ^= data[count];
	        count++;
	        for (uint8_t i = 0; i < 8; i++)
	            crc = crc & 0x80 ? (crc << 1) ^ 0x07 : crc << 1;
	    }                                                            //Окончание вычисления CRC-8

	if(crc == data[data[4]+7])                                       //Далее обработка проверки CRC-8 принятого пакета
		                                                             //адреса отправителя, приёмника.
	{
		crc = 0;
		if(source == data[5])
		{
			if(receiver == data[6])
			{
		port_rx.rx_data[0] = data[7];
		port_rx.rx_data[1] = data[8];
		port_rx.rx_data[2] = data[9];
		port_rx.rx_data[3] = data[10];
		GPIOE->ODR = port_rx.port_rx;
			}
			else
		    return 3; //Если адрес приёмника не совпал
		}
		else
		return 2; //Если адрес отправителя не совпал
	}
	else
	return 1;  //Возвращиет если CRC-8 не совпала

	return 0; //Если пакет полон и соответствует формату данных
}

int main(void)
{
   HAL_UART_Receive_IT(&huart1, tmp_buf, 1);                    //Запуск ожидания приёма USART в прерывании
  
  while (1)
  {
	  if(trig)
	  {
		  trig=0;
		  buff[cout_rx] = tmp_buf[0];                         //заполняет массив buff[] который нужен для передачи пакета в
		                                                      //функцию Rx_Function(0x11, 0x22,buff);
			   cout_rx++;
			   HAL_UART_Receive_IT(&huart1, tmp_buf, 1);      //Запуск ожидания приёма USART в прерывании
			   if(cout_rx > 11)                               //Когда массив buff[] сформирован полностью
			   {
				   HAL_TIM_Base_Stop_IT(&htim11);             //Останов таймер 11
				   TIM11->CNT = 0;                            //Обнуления счётчика таймера 11
				   cout_rx = 0;                               //Обнуление счётчика массива buff[]
				   GPIOD->ODR = Rx_Function(0x11, 0x22,buff); //Вызов функции обработки принятого пакета
			   }
	  }

  }
}
