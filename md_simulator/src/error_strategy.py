'''
Created on Oct 08, 2014

@author: ctait
'''
from __future__ import print_function

import random
import sys

class ErrorStrategies:
    class RandomStrategy(object):
      
        def __init__(self,errors):
            self.errors = errors
            self.dup_error=0
            self.best_sell_le_best_buy_error=0
            self.no_order_for_modify=0
            self.no_order_for_trade_error=0
            self.order_range_error=0
            self.order_parse_error=0
            self.side_error=0
            self.side_error=0
            self.quantity_range_error=0
            self.quantity_parse_error=0
            self.price_range_error=0
            self.price_parse_error=0
            self.corruption_error=0
            
        ################################################################################
        # Error generators
        ################################################################################
        def gen_dup_add(self,dup_order_id,side,quantity,price):
            # 1 in 1000 
            if self.errors and random.randint(1,1000) == 1:
                message='{0:d},{1},{2:d},{3:.0f}'.format(dup_order_id,side,quantity,price)
                print('A,'+message)
                self.dup_error += 1
                
        def gen_bad_cancel(self,side,quantity,price):
            # 1 in 1000
            if self.errors and random.randint(1,1000) == 1:
                order_id=1
                message='{0:d},{1},{2:d},{3:.0f}'.format(order_id,side,quantity,price)
                print('X,'+message, file=sys.stdout)
                self.no_order_for_modify += 1
                
        def gen_order_range_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                message='{0:d},{1},{2:d},{3:.0f}'.format(-new_order_id,side,quantity,price)
                print('A,'+message,file=sys.stdout)
                self.order_range_error += 1
                
        def gen_order_parse_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                bad_order_id="ABDC"
                message='{0},{1},{2:d},{3:.0f}'.format(bad_order_id,side,quantity,price)
                print('A,'+message,file=sys.stdout)
                self.order_parse_error += 1
                
        def gen_side_value_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                side="C"
                message='{0:d},{1},{2:d},{3:.0f}'.format(new_order_id,side,quantity,price)
                print('A,'+message,file=sys.stdout)
                self.side_error += 1
                
        def gen_side_parse_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                side=""
                message='{0:d},{1},{2:d},{3:.0f}'.format(new_order_id,side,quantity,price)
                print('A,'+message,file=sys.stdout)
                self.side_error += 1
                
        def gen_quantity_range_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                message='{0:d},{1},{2:d},{3:.0f}'.format(new_order_id,side,-quantity,price)
                print('A,'+message,file=sys.stdout)
                self.quantity_range_error += 1
    
        def gen_quantity_parse_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                bad_quantity="ABDC"
                message='{0:d},{1},{2},{3:.0f}'.format(new_order_id,side,bad_quantity,price)
                print('A,'+message,file=sys.stdout)
                self.quantity_parse_error += 1
                
        def gen_price_range_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                message='{0:d},{1},{2:d},{3:.0f}'.format(new_order_id,side,quantity,-price)
                print('A,'+message,file=sys.stdout)
                self.price_range_error += 1
    
        def gen_price_parse_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                bad_price="ABDC"
                message='{0:d},{1},{2:d},{3}'.format(new_order_id,side,quantity,bad_price)
                print('A,'+message,file=sys.stdout)
                self.price_parse_error += 1
                
        def gen_corruption_error(self,new_order_id,side,quantity,price):
            # 1 in 5000
            if self.errors and random.randint(1,5000) == 1:
                message='{0:d},{1},{2:d},{3}'.format(new_order_id,side,quantity,price)
                print(','+message,file=sys.stdout)
                self.corruption_error += 1
                
        def gen_bogus_add_order(self,side,quantity,price,gen_order_id,flip_side,ob):
            # Here we can generate a bogus add order which causes a 
            # match with no trades occuring error.
            if self.errors and random.randint(1,500) == 1:
                error_order_id=gen_order_id()
                best_sell_le_best_buy_message='{0:d},{1},{2:d},{3:.0f}'.format(error_order_id,flip_side(side),1,price)
                ob.add(best_sell_le_best_buy_message)
                print('A,'+best_sell_le_best_buy_message)
                self.best_sell_le_best_buy_error += 1
            
        def gen_bogus_trade(self,side,quantity,price,tick_size):
            # Here we can generate a bogus trade with no corresponding order
            if self.errors and random.randint(1,500) == 1:
                no_order_for_trade_message='{0:d},{1:.0f}'.format(1,price-tick_size)
                print('T,'+no_order_for_trade_message)
                self.no_order_for_trade_error += 1
            
        ################################################################################
