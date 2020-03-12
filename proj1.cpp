#include "proj1.hpp"

namespace eosio {

void token::create( name   brokerage,
                    asset  max_supply )
{
    require_auth( _self );
    require_auth(brokerage);
    

    auto sym = max_supply.symbol;

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing == statstable.end(), "duplicated token" );

    statstable.emplace( _self, [&]( auto& s ) {
       s.supply.symbol = max_supply.symbol;
       s.max_supply    = max_supply;
       s.brokerage     = brokerage;
    });
}


void token::issue( name to, asset quantity, int order_id )
{
   require_auth(to);
    auto sym = quantity.symbol;

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    eosio_assert( existing != statstable.end(), "token does not exist" );
    const auto& st = *existing;

    require_auth( st.brokerage );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.brokerage, quantity, st.brokerage );

    if( to != st.brokerage ) {
      SEND_INLINE_ACTION( *this, transfer, { {st.brokerage, "active"_n} },
                          { st.brokerage, to, quantity, order_id }
      );
    }
}

void token::transfer( name    from,
                      name    to,
                      asset   quantity,
                      int  order_id )
{

    require_auth( from );
    require_auth( to );

    auto sym = quantity.symbol.code();
    stats statstable( _self, sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( name account_owner, asset amount ) {
   accounts from_acnts( _self, account_owner.amount );

   const auto& from = from_acnts.get( amount.symbol.code().raw(), "no balance object found" );

   from_acnts.modify( from, account_owner, [&]( auto& a ) {
         a.balance -= amount;
      });
}

void token::add_balance( name account_owner, asset amount, name ram_payer ) //important
{
   accounts to_acnts( _self, account_owner.amount );
   auto to = to_acnts.find( amount.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = amount;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += amount;
      });
   }
}

}

EOSIO_DISPATCH( eosio::token, (create)(issue)(transfer) )
