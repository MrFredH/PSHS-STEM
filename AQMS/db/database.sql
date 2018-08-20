create table unit_type (
	rec_id		SERIAL primary key,
	create_ts	timestamp default now(),
	name		text
);
insert into unit_type (name) values ('Geometry');
insert into unit_type (name) values ('Time');
insert into unit_type (name) values ('Mass');
insert into unit_type (name) values ('Electricity');
insert into unit_type (name) values ('Temperature');
insert into unit_type (name) values ('Amount of substance');
insert into unit_type (name) values ('Electro Magnetic');

create table unit (
	rec_id		SERIAL primary key,
	create_ts	timestamp default now(),
	unit_type 	integer	REFERENCES unit_type (rec_id),
	short_unit_name	text,
	long_unit_name	text
);
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Geometry'), '°', 'angle');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Geometry'), 'm', 'meter');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Time'), 's', 'second');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Mass'), 'g', 'gram');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Electricity'), 'A', 'ampere');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Electricity'), 'V', 'volt');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Electricity'), 'R', 'resistance');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Electricity'), 'F', 'farad');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Temperature'), 'K', 'kelvin');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Temperature'), 'C', 'celcius');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Temperature'), 'F', 'farenheight');
insert into unit (unit_type, short_unit_name, long_unit_name) values ((select rec_id from unit_type where name='Amount of substance'), 'mol', 'mole');

create table prefix (
		rec_id		SERIAL primary key,
		create_ts	timestamp default now(),
		name		text,
		prefix		varchar(8),
		power		integer
);
insert into prefix (name, power, prefix) values ('yotta', 24, 'Y');
insert into prefix (name, power, prefix) values ('zetta', 21, 'Z');
insert into prefix (name, power, prefix) values ('exa', 18, 'E');
insert into prefix (name, power, prefix) values ('peta', 15, 'P');
insert into prefix (name, power, prefix) values ('tera', 12, 'T');
insert into prefix (name, power, prefix) values ('giga', 9, 'G');
insert into prefix (name, power, prefix) values ('mega', 6, 'M');
insert into prefix (name, power, prefix) values ('kilo', 3, 'k');
insert into prefix (name, power, prefix) values ('hecto', 2, 'h');
insert into prefix (name, power, prefix) values ('deka', 1, 'da');
insert into prefix (name, power, prefix) values ('deci', -1, 'd');
insert into prefix (name, power, prefix) values ('centi', -2, 'c');
insert into prefix (name, power, prefix) values ('milli', -3, 'm');
insert into prefix (name, power, prefix) values ('micro', -6, 'µ');
insert into prefix (name, power, prefix) values ('nano', -9, 'n');
insert into prefix (name, power, prefix) values ('pico', -12, 'p');
insert into prefix (name, power, prefix) values ('famto', -15, 'f');
insert into prefix (name, power, prefix) values ('atto', -18, 'a');
insert into prefix (name, power, prefix) values ('zepto', -21, 'z');
insert into prefix (name, power, prefix) values ('yocto', -24, 'y');

create table sensor_type (
		rec_id		SERIAL primary key,
		create_ts	timestamp default now(),
		name		text
);

insert into sensor_type (name) values ('pH');
insert into sensor_type (name) values ('Temperature');
insert into sensor_type (name) values ('Liquid Flow');

create table sensor (
		rec_id		SERIAL primary key,
		create_ts	timestamp default now(),
		name		text,
		location	text,
		project		text,
		sensor_address  text,
		sensor_contact  text,
		sensor_type	integer REFERENCES sensor_type (rec_id)
);

create table sensor_data (
		rec_id		SERIAL primary key,
		create_ts	timestamp default now(),
		report_ts	timestamp,		
		sensor_id	integer REFERENCES sensor (rec_id),
		value		numeric,
		prefix		integer REFERENCES prefix (rec_id),
		unit		integer REFERENCES unit (rec_id)
);

