#pragma once
#include <stdio.h>
#include <fstream>
#include "predefined.h"
#include "user.h"
#include "train.h"
#include "ticket.h"
#include "bptree.h"
#include "string.h"

#define user_block sizeof(User)

int user_number = 0;
FILE *fp;

void file_register(User &user) {
	fseek(fp, 0, SEEK_END);
	user.intid = 2018 + user_number;
	user.id = IntToString<20>(user.intid);
	if (user.intid == 2018) user.privilege = 2;
	else user.privilege = 1;
	fwrite(&user, sizeof(User), 1, fp);
	fflush(fp);
	++user_number;
	cout << user.id << '\n';
	return;
}

void file_login(const int cintid, const mystring<20> &cpassword) {
	if (cintid < 2018 || cintid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	User user;
	fseek(fp, (cintid - 2018) * user_block, SEEK_SET);
	fread(&user, sizeof(User), 1, fp);
	if (user.password == cpassword) cout << 1 << '\n';
	else cout << 0 << '\n';
	return;
}

void file_query_profile(const int cintid) {
	if (cintid < 2018 || cintid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	fseek(fp, (cintid - 2018) * user_block, SEEK_SET);
	User user;
	fread(&user, sizeof(User), 1, fp);
	cout << user.name << ' ' << user.email << ' ' << user.phone << ' ' << user.privilege << '\n';
	return;
}

void file_modify_profile(const int cintid, const mystring<40> &cname, const mystring<20> &cpassword, const mystring<20> &cemail, const mystring<20> &cphone) {
	if (cintid < 2018 || cintid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	User user;
	fseek(fp, (cintid - 2018) * user_block, SEEK_SET);
	fread(&user, sizeof(User), 1, fp);
	user.intid = cintid;
	user.id = IntToString<20>(user.intid);
	user.name = cname;
	user.password = cpassword;
	user.email = cemail;
	user.phone = cphone;
	fseek(fp, (cintid - 2018) * user_block, SEEK_SET);
	fwrite(&user, sizeof(User), 1, fp);
	fflush(fp);
	cout << 1 << '\n';
	return;
}

void file_modify_priviledge(const int &id1, const int &id2, const int &privilege) {
	if (id1 < 2018 || id1 >= 2018 + user_number || id2 < 2018 || id2 >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	User user1, user2;
	fseek(fp, (id1 - 2018) * user_block, SEEK_SET);
	fread(&user1, sizeof(User), 1, fp);
	fseek(fp, (id2 - 2018) * user_block, SEEK_SET);
	fread(&user2, sizeof(User), 1, fp);
	if (user1.privilege == 2 && user2.privilege != 2) {
		user2.privilege = privilege;
		user2.intid = id2;
		user2.id = IntToString<20>(user2.intid);
		fseek(fp, (id2 - 2018) * user_block, SEEK_SET);
		fwrite(&user2, sizeof(User), 1, fp);
		fflush(fp);
		cout << 1 << '\n';
	}
	else if (user1.privilege == 2 && user2.privilege == 2 && privilege == 2) cout << 1 << '\n';
	else cout << 0 << '\n';
	return;
}

void file_buy_ticket(const mystring<20> &id, const int &intid, const int &num, const mystring<20> &train_id,
	const mystring<20> &loc1, const mystring<20> &loc2, const mystring<10> &date,
	const int &intdate, const mystring<20> &ticket_kind, bplus_tree<train> &thetrain,
	bplus_tree<ticket> &theticket, bplus_tree<char> &find_ticket)
{
	if (intid < 2018 || intid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	train pos;
	ticket findticket;
	if (thetrain.search(train_id, &pos) != 0) {
		cout << 0 << '\n';
		return;
	}
	if (pos.buy(num, loc1, loc2, intdate, ticket_kind)) {
		thetrain.update(train_id, pos);
		mystring<90> key = id + date + train_id + loc1 + loc2;
		if (theticket.search(key, &findticket) != 0) {
			int loc1_pos, loc2_pos;
			for (loc1_pos = 0; loc1_pos < pos.num_station - 1; ++loc1_pos) {
				if (pos.sta[loc1_pos].name == loc1)
					break;
			}
			for (loc2_pos = loc1_pos + 1; loc2_pos < pos.num_station; ++loc2_pos) {
				if (pos.sta[loc2_pos].name == loc2)
					break;
			}
			int date_from = intdate + pos.sta[loc1_pos].day_out_offset;
			int date_to = intdate + pos.sta[loc2_pos].day_in_offset;
			int month_from = (date_from > 30) ? 7 : 6;
			int month_to = (date_to > 30) ? 7 : 6;
			if (date_from > 30)
				date_from -= 30;
			if (date_to > 30)
				date_to -= 30;
			mystring<10> ret_date_loc1("2018-0"), ret_date_loc2("2018-0");
			ret_date_loc1 += ('0' + month_from);
			ret_date_loc1 += '-';
			ret_date_loc2 += ('0' + month_to);
			ret_date_loc2 += '-';
			ret_date_loc1 += ('0' + date_from / 10);
			ret_date_loc1 += ('0' + date_from % 10);
			ret_date_loc2 += ('0' + date_to / 10);
			ret_date_loc2 += ('0' + date_to % 10);
			ticket newticket(train_id, loc1, loc2, date, pos.catalog, ret_date_loc1, pos.sta[loc1_pos].start, ret_date_loc2, pos.sta[loc2_pos].arrive, pos.num_price);
			for (int i = 0; i < newticket.num_kind; ++i) {
				newticket.ticket_kind[i] = pos.name_price[i];
				for (int j = loc1_pos; j <= loc2_pos; ++j) {
					newticket.price_ticket_kind[i] += pos.sta[j].price[i];
				}
			}
			for (int i = 0; i < newticket.num_kind; ++i) {
				if (newticket.ticket_kind[i] == ticket_kind) {
					newticket.num_ticket_kind[i] += num;
					break;
				}
			}
			theticket.insert(key, newticket);
			find_ticket.insert(key, newticket.catalog);
			cout << 1 << '\n';
			return;
		}
		else {
			for (int i = 0; i < findticket.num_kind; ++i) {
				if (findticket.ticket_kind[i] == ticket_kind) {
					findticket.num_ticket_kind[i] += num;
					theticket.update(key, findticket);
					cout << 1 << '\n';
					return;
				}
			}
		}
	}
	else {
		cout << 0 << '\n';
	}
	return;
}

void file_query_order(const mystring<20> &id, const int &intid, const mystring<10> &date, const mystring<10> &catalog,
	bplus_tree<ticket> &theticket, bplus_tree<char> &find_ticket) {
	if (intid < 2018 || intid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	mystring<60> left, right;
	for (int i = 0; i < 60; ++i) {
		left += ' ';
		right += 'z';
	}
	mystring<90> leftkey = id + date + left;
	mystring<90> rightkey = id + date + right;
	sjtu::vector<bplus_tree<char>::record_t> key_sequence;
	sjtu::vector<bplus_tree<char>::record_t> true_sequence;

	find_ticket.search_range(leftkey, rightkey, key_sequence);
	for (int i = 0; i < key_sequence.size(); ++i) {
		for (int j = 0; j < catalog.len; ++j) {
			if (key_sequence[i].value == catalog.str[j]) {
				true_sequence.push_back(key_sequence[i]);
				break;
			}
		}
	}
	if (true_sequence.empty()) {
		cout << -1 << '\n';
		return;
	}
	cout << true_sequence.size() << '\n';
	for (int i = 0; i < true_sequence.size(); ++i) {
		mystring<90> findkey;
		for (int j = 0; j < 90; ++j) {
			findkey += true_sequence[i].key.k[j];
		}
		ticket pos;
		theticket.search(findkey, &pos);
		pos.display();
	}
	return;
}

void file_refund_ticket(const int &num, const mystring<20> &id, const int &intid, const mystring<20> &train_id,
	const mystring<20> &loc1, const mystring<20> &loc2, const mystring<10> &date, const int &intdate,
	const mystring<20> &ticket_kind, bplus_tree<ticket> &theticket, bplus_tree<train> &thetrain) {
	if (intid < 2018 || intid >= 2018 + user_number) {
		cout << 0 << '\n';
		return;
	}
	train postrain;
	ticket pos;
	if (thetrain.search(train_id, &postrain) != 0) {
		cout << 0 << '\n';
		return;
	}
	mystring<90> key = id + date + train_id + loc1 + loc2;
	if (theticket.search(key, &pos) != 0) {
		cout << 0 << '\n';
		return;
	}
	if (postrain.refund(num, loc1, loc2, intdate, ticket_kind)) {
		thetrain.update(train_id, postrain);
		for (int i = 0; i < pos.num_kind; ++i) {
			if (pos.ticket_kind[i] == ticket_kind) {
				if (pos.num_ticket_kind[i] < num) {
					cout << 0 << '\n';
					return;
				}
				else {
					pos.num_ticket_kind[i] -= num;
					theticket.update(key, pos);
					cout << 1 << '\n';
					return;
				}
			}
		}
	}
	else cout << 0 << '\n';
	return;
}