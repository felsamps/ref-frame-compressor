#include <vector>

#include "../inc/Quantizer.h"

double abs(double l0) {
	return (l0 < 0) ? -l0 : l0;
}

Pel closestToZero(double l0, double l1) {
	if((l0 > 0 && l1 < 0) || (l0 < 0 && l1 > 0)) {
		return 0;
	}
	return abs(l0) < abs(l1) ? l0 : l1;
}

Quantizer::Quantizer(string fileName) {
	this->quantFile.open(fileName.c_str(), fstream::in);
	this->levels.clear();
	this->fQuant.clear();
	this->iQuant.clear();

	xParseQuantFile();
	xGenerateQuantTable();
}

void Quantizer::xParseQuantFile() {
	this->levels.push_back(-129.00);
	while(!this->quantFile.eof()) {
		double level;
		this->quantFile >> level;
		this->levels.push_back(level);
	}
	this->levels.pop_back();
	this->levels.push_back(128.00);

	this->numOfLevels = this->levels.size() - 1;
}

void Quantizer::xGenerateQuantTable() {
	Pel l = -(this->numOfLevels/2);
	int idx = 0;
	for (int sample = -128; sample < 128; sample++) {
		if((double)sample >= this->levels[idx+1]) {
			l++;
			idx++;
		}		
		this->fQuant[(Pel)sample] = l;
	}
	idx = 0;
	for (Pel sample = -(this->numOfLevels/2); sample <= (this->numOfLevels/2); sample ++) {
		this->iQuant[sample] = closestToZero(this->levels[idx], this->levels[idx+1]);
		idx++;
		
	}
}

Pel** Quantizer::quantize(Pel** residue, int size, int xx, int yy) {
	Pel** error = new Pel*[size];
	for (int i = 0; i < size; i++) {
		error[i] = new Pel[size];
	}

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			Pel quantized = this->fQuant[residue[x+xx][y+yy]];
			error[x][y] = residue[x+xx][y+yy] - iQuant[quantized];
			residue[x+xx][y+yy] = quantized;
		}
	}
	return error;
}

Pel** Quantizer::getError(Pel** residue, int size, int xx, int yy) {
	Pel** error = new Pel*[size];
	for (int i = 0; i < size; i++) {
		error[i] = new Pel[size];
	}

	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			Pel quantized = this->fQuant[residue[x+xx][y+yy]];
			error[x][y] = residue[x+xx][y+yy] - iQuant[quantized];
		}
	}
	return error;
}

void Quantizer::invQuantize(Pel** quantized, int size,  int xx, int yy) {
	for (int y = 0; y < size; y++) {
		for (int x = 0; x < size; x++) {
			quantized[x+xx][y+yy] = this->iQuant[quantized[x+xx][y+yy]];
		}
	}
}

void Quantizer::report() {
	for (int i = 0; i < this->levels.size(); i++) {
		cout << this->levels[i] << " ";
	}
	cout << endl;

	for (int sample = -128; sample < 128; sample++) {
		cout << sample << " " << (int)this->fQuant[(Pel)sample] << endl;
	}
	for (Pel sample = -(this->numOfLevels/2); sample <= (this->numOfLevels/2); sample ++	) {
		cout << (int)sample << " " << (int)this->iQuant[sample] << endl;
	}
}

int Quantizer::getNumOfLevels() {
	return this->numOfLevels;
}