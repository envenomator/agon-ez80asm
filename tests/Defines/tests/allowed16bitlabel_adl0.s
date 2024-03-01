	assume adl=0
	org $0
	dw shortlabel
shortlabel:
	assume adl=1
	org $40000
label: