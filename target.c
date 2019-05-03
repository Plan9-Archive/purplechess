
void
cleartargs(void)
{
	int i;

	for(i = 0; i < 64; i++)
		pos->sq[i] &= ~TARGET;
}

int
target(int n)
{
	char *ps;

	ps = pos->sq + n;
	if(!(pos->n & 1) && *ps & WHITE || pos->n & 1 && *ps & BLACK)
		return 1;	/* blocked by own piece */
	*ps |= TARGET;
	if(*ps & PC)
		return 1;	/* capture available */
	return 0;		/* empty square */
}

void
wptargs(int n)
{
	char *ps;

	ps = pos->sq + n;
	if(*(ps + 8) == NOPIECE){
		if(n / 8 == 1 && *(ps + 16) == NOPIECE)
			target(n + 16);
		target(n + 8);
	}
	if(n % 8 && (*(ps + 7) || n + 7 == pos->eptarg))
		target(n + 7);
	if((n + 1) % 8 && (*(ps + 9) || n + 9 == pos->eptarg))
		target(n + 9);
}

void
bptargs(int n)
{
	char *ps;

	ps = pos->sq + n;
	if(*(ps - 8) == NOPIECE){
		if(n / 8 == 6 && *(ps - 16) == NOPIECE)
			target(n - 16);
		target(n - 8);
	}
	if((n + 1) % 8 && (*(ps - 7) || n - 7 == pos->eptarg))
		target(n - 7);
	if(n % 8 && (*(ps - 9) || n - 9 == pos->eptarg))
		target(n - 9);
}

void
knighttargs(int n)
{
	if(n > 15 && n % 8)
		target(n - 17);
	if(n > 15 && (n + 1) % 8)
		target(n - 15);
	if(n > 7 && n % 8 > 1)
		target(n - 10);
	if(n > 7 && (n + 2) % 8 > 1)
		target(n - 6);
	if(n < 56 && n % 8 > 1)
		target(n + 6);
	if(n < 56 && (n + 2) % 8 > 1)
		target(n + 10);
	if(n < 48 && n % 8)
		target(n + 15);
	if(n < 48 && (n + 1) % 8)
		target(n + 17);
}

int
canOO(int n)
{
	if(!(pos->castling & (pos->n & 1 ? BOO : WOO)))
		return 0;	/* already lost rights */
	if(pos->sq[n + 1] || pos->sq[n + 2])
		return 0;	/* rank blocked */
	return 1;
}

int
canOOO(int n)
{
	if(!(pos->castling & (pos->n & 1 ? BOOO : WOOO)))
		return 0;	/* already lost rights */
	if(pos->sq[n - 1] || pos->sq[n - 2] || pos->sq[n - 3])
		return 0;	/* rank blocked */
	return 1;
}

void
kingtargs(int n)
{
	if(canOO(n))
		target(n + 2);
	if(canOOO(n))
		target(n - 2);	
	if(n < 56)
		target(n + 8);
	if(n > 7)
		target(n - 8);
	if((n + 1) % 8){
		target(n + 1);
		if(n < 56)
			target(n + 9);
		if(n > 7)
			target(n - 7);
	}
	if(n % 8){
		target(n - 1);
		if(n < 56)
			target(n + 7);
		if(n > 7)
			target(n - 9);
	}
}

void
recttargs(int n){
	int i;

	for(i = n + 1; i % 8; i++)
		if(target(i))
			break;
	for(i = n - 1; (i + 1) % 8; i--)
		if(target(i))
			break;
	for(i = n + 8; i < 64; i += 8)
		if(target(i))
			break;
	for(i = n - 8; i >= 0; i -= 8)
		if(target(i))
			break;
}

void
diagtargs(int n){
	int i;

	for(i = n + 7; (i + 1) % 8 && i < 64; i += 7)
		if(target(i))
			break;
	for(i = n + 9; i % 8 && i < 64; i += 9)
		if(target(i))
			break;
	for(i = n - 7; i % 8 && i >= 0; i -= 7)
		if(target(i))
			break;
	for(i = n - 9; (i + 1) % 8 && i >= 0; i -= 9)
		if(target(i))
			break;
}

void
findtargs(int n)
{
	switch(pos->sq[n] & PC){
	case PAWN:
		if(pos->n & 1)
			bptargs(n);
		else
			wptargs(n);
		break;
	case KNIGHT:
		knighttargs(n);
		break;
	case BISHOP:
		diagtargs(n);
		break;
	case ROOK:
		recttargs(n);
		break;
	case QUEEN:
		recttargs(n);
		diagtargs(n);
		break;
	case KING:
		kingtargs(n);
		break;
	}
	pos->sq[n] |= TARGET;
}
