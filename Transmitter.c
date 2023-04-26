union
{
uint8_t tx_data[4];
uint32_t port_tx;
}port_tx;

uint8_t Tx_Function(uint16_t length, uint8_t source, uint8_t receiver, uint8_t *data)
{
	if(length > 256)
		return 0;
	uint8_t crc = 0;
	uint8_t count=0;
	uint8_t count_crc = 0;
	uint8_t priambula_tx[256] = {0x01, 0x02, 0x03, 0x04};

		priambula_tx[4] = length;
		priambula_tx[5] = source;
		priambula_tx[6] = receiver;


for(count=7; count <  length+7; count++)
{
	priambula_tx[count] = data[count-7];

}
count=0;
count_crc=length+6;
while (count_crc--)
    {
        crc ^= priambula_tx[count];
        count++;
        for (uint8_t i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x07 : crc << 1;
    }
priambula_tx[length+7] = crc;
HAL_UART_Transmit(&huart1, priambula_tx, length+8,100);

	return length+8;

}

int main(void)
{
while (1)
{
    if(port_tx.port_tx != GPIOB->IDR)
{
	port_tx.port_tx = GPIOB->IDR;
	Tx_Function(sizeof(port_tx.tx_data) ,0x11, 0x22, port_tx.tx_data);
}
}
}
