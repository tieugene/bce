#include <iostream>
#include "out/log.h"
#include "bce.h"
#include "misc.h" // memused

using namespace std;

const std::string TAB = "\t";

void log_opts(void) {
  cerr
      << "= Options: =" << endl
      << "From:" << TAB << ((OPTS.from == MAX_UINT32) ? string("<not set>") : to_string(OPTS.from)) << endl
      << "Num:" << TAB << OPTS.num << endl
      << "Dat dir:" << TAB << OPTS.datdir << endl
      << "Locs file:" << TAB << OPTS.locsfile << endl
      << "Cin:" << TAB << OPTS.fromcin << endl
      << "K-V dir:" << TAB << OPTS.kvdir << endl
      << "K-V type:" << TAB << OPTS.kvngin << endl
      << "K-V tune:" << TAB << OPTS.kvtune << endl
      << "Out:" << TAB << OPTS.out << endl
      << "Log by:" << TAB << OPTS.logstep << endl
      << "Debug:" << TAB << OPTS.verbose << endl
      << "M/t:" << TAB << OPTS.mt << endl
      << endl;
      ;
}

void    log_head(void) {
  cerr << "Bk\tTx\tVins\tVouts\tAddrs\tUAddrs\tMem,M\tTime\n";
  log_tail();
}

void    log_tail(void) {
  cerr << "------\t-------\t-------\t-------\t-------\t-------\t-------\t-----\n";
}

void    log_interim(void) {
  cerr <<
    COUNT.bk+1 <<
    TAB << COUNT.tx <<
    TAB << STAT.vins <<
    TAB << STAT.vouts <<
    TAB << STAT.addrs <<
    TAB << COUNT.addr <<
    TAB << ((memused() - start_mem) >> 10) <<
    TAB << time(nullptr) - start_time <<
    endl;
}

void    log_summary(void)
{
  cerr
    << "= Summary =" << endl
    << "Blocks:"    << TAB << COUNT.bk+1 << endl
    << "Tx:"        << TAB << COUNT.tx << endl
    << "Vins:"      << TAB << STAT.vins << endl
    << "Vouts:"     << TAB << STAT.vouts << endl
    << "Addrs:"     << TAB << STAT.addrs << endl
    << "Max tx/bk:" << TAB << STAT.max_txs << endl
    << "Max vi/tx:" << TAB << STAT.max_vins << endl
    << "Max vo/tx:" << TAB << STAT.max_vouts << endl
    << "Max ad/vo:" << TAB << STAT.max_addrs << endl
    << "Addr lengths:" << endl;
  for (auto i = 0; i < 321; i++)
    if (STAT.addr_lens[i])
      cerr << i << " : " << STAT.addr_lens[i] << endl;
}
