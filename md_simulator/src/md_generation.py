'''
Created on Oct 08, 2014

@author: ctait
'''
from __future__ import print_function

import order_book
import simulation_strategy
import error_strategy
        
class MDGenerator(object):
    """
    This is the generator for the book based on the specification
    The generator creates events to mimic a book including-
    1. Adding orders
    2. Modifying orders
    3. Cancelling orders
    4. Manipulating the book so that is causes a price movement with
       trades and all the consistent modifications and cancellation
       addition adds to stabilise the the book as if it was real
    5. Allows the addition of error events which cover the specification
       requirement laid out in item 5
       
    What make this possible is actually using a prototype of the order book itself
    which can process all these events correctly.
    
    So part of the generator makeup is to use the OrderBook object to assist with
    choosing the correct adds and cancels and also choices in the price
    movement. 
    """
    def __init__(self,
                 errors,
                 strategy,
                 start_price=900.00,
                 num_levels=5,
                 tick_size=25,
                 bo_spread=2,
                 start_order_id=1000000,
                 max_orders=20,
                 max_order_quantity=10,
                 tick_margin=8):
        self.errors=errors
        self.ob = order_book.OrderBook()
        self.start_price=start_price
        self.num_levels=num_levels
        self.tick_size=tick_size
        self.bo_spread=bo_spread
        self.start_order_id=start_order_id
        self.max_orders=max_orders
        self.max_order_quantity=max_order_quantity
        self.last_order_id=1000000
        self.tick_margin=tick_margin
        self.bid_limit=self.start_price-self.tick_margin*self.tick_size
        self.ask_limit=self.start_price+self.tick_margin*self.tick_size
        targetClass = getattr(simulation_strategy.SimulationStrategies,strategy)
        self.strategy = targetClass(self.ob,self.max_order_quantity,self.tick_size)
        targetClass = getattr(error_strategy.ErrorStrategies,strategy)
        self.error_strategy = targetClass(self.errors)
        
    def create_order_book(self):
        """
        Initial creation of the order book
        """
        # Buy side
        for i in range(0,5):
            self.print_add(self.gen_order_id(),'B',self.max_order_quantity/2,self.start_price+self.tick_size*i)
            
        # Sell side
        for i in range(6,11):
            self.print_add(self.gen_order_id(),'S',self.max_order_quantity/2,self.start_price+self.tick_size*i)
        
    def generate_book_events(self,number_of_changes):
        """
        Main generation method uses strategy choices from a distribution below
        """
                
        # From number of changes generate
        for ci in range(number_of_changes):
            event=self.strategy.choose_event()
            (side,price)=self.strategy.choose_price()
            if event=='X':
                # Generate the modify event
                self.gen_cancel(side,price)
            elif event=='M':
                # Generate the modify event
                self.gen_modify(side,price)
            elif event=='A':
                # Generate the add event
                self.gen_add(side,price)
            elif event=='T':
                # Generate price move event
                self.gen_price_move(side,price)
        
    ################################################################################
    # Main event methods
    ################################################################################
    def gen_add(self,side,price):
        """
        Create the event
        """
        price_orders=self.get_price_orders(side,price)
        if len(price_orders) < 10:
            new_order_id=self.last_order_id+1
            self.last_order_id=new_order_id
            quantity=self.strategy.choose_quantity()
            self.print_add(new_order_id,side,quantity,price)
            # Error events may be added if requested
            self.error_strategy.gen_dup_add(new_order_id,side,quantity,price)
            self.error_strategy.gen_order_range_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_order_parse_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_side_value_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_side_parse_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_quantity_range_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_quantity_parse_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_price_range_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_price_parse_error(new_order_id,side,quantity,price)
            self.error_strategy.gen_corruption_error(new_order_id,side,quantity,price)
        
    def print_add(self,order_id,side,quantity,price):
        message='{0:d},{1},{2:d},{3:.0f}'.format(order_id,side,quantity,price)
        self.ob.add(message)
        print('A,'+message)
            
    def gen_modify(self,side,price):
        # Choose the order you want to modify
        price_orders=self.get_price_orders(side,price)
        (order_id,quantity)=self.strategy.choose_order(price_orders)
        new_quantity=self.strategy.choose_quantity(quantity)
        self.print_modify(order_id,side,new_quantity,price)
        
    def print_modify(self,order_id,side,quantity,price):
        message='{0:d},{1},{2:d},{3:.0f}'.format(order_id,side,quantity,price)
        self.ob.modify(message)
        print('M,'+message)
        
    def gen_cancel(self,side,price):
        # Choose the order you want to cancel
        # Get the quantity
        if side=='B':
            price_levels=self.ob.buyLevels
        else:
            price_levels=self.ob.sellLevels
        (order_id,quantity)=self.strategy.choose_order(price_levels[price])
        lpo=len(price_levels[price])
        if lpo>1:
            self.print_cancel(order_id,side,quantity,price)
            self.error_strategy.gen_bad_cancel(side,quantity,price)
            
    def print_cancel(self,order_id,side,quantity,price):
        message='{0:d},{1},{2:d},{3:.0f}'.format(order_id,side,quantity,price)
        self.ob.cancel(message)
        print('X,'+message)
    ################################################################################
        
    ################################################################################
    # Price movement generation
    ################################################################################
    def gen_price_move(self,side,price):
        '''
        Generate the price movement by preparing how we can move and creating the
        the move based on that.
        
        Sort the prices and then look at all the trades you need to make against the
        matched orders to generate a match event, we can allow partials as well
        
        Follow up the match trade is creating a demand
        If sell we are bound to drive price down and up 
        if buy so add another order with the match
        properties which reflects this. Keep note that
        if we go partial fill then we will only add if
        there is a free space at top of match side where
        we would not cause a fill so check top of book
        at opposite side to make sure
           
        We can have a few different scenarios here -
        1. Remove 1-3 tick level(s) only - this can be simple case
        2. Say remove 3 ticks on bid so say -
        we fill (remove first 2 levels and then partial
        the third
        3. Partial 1 tick level - this means cancel
        the match order but we don't necessarily need to add and
        can optionally do this as it would cause no harm
        4. What if we can't match the whole order then we should
        not cancel the match order but modify the match order
        quantity
        '''
        if self.allow_price_move(side):
            tick_move=self.strategy.choose_tick_move()
            if side=='B':
                price_levels=sorted(self.ob.buyLevels.items(), key=lambda t: t[0], reverse=True)
            else:
                price_levels=sorted(self.ob.sellLevels.items(), key=lambda t: t[0], reverse=False)
            
            trade_messages=[]
            cancel_messages=[]
            modify_messages=[]
            move_length=min(len(price_levels),tick_move)
            (match_price,last_level) = price_levels[move_length-1]
            match_order_quantity=0

            match_order_quantity=self.create_price_move_message(move_length,price_levels,tick_move,side,match_price,trade_messages,modify_messages,cancel_messages)
           
            self.gen_price_move_messages(self.gen_order_id(),
                                    flip_side(side),
                                    match_order_quantity,
                                    match_price,
                                    trade_messages,cancel_messages,modify_messages)
            
            self.stabilize_order_book(side,match_price)
            

    def create_price_move_message(self,move_length,price_levels,tick_move,side,match_price,trade_messages,modify_messages,cancel_messages):
        """
        Create the price movement now
        I have implemented the following cases
        
        Case 1. Always match a complete level
        Case 2. Partial the last if we are removing 2 or 3 levels partial the last
        """
        match_case=self.strategy.choose_match_case()
        match_order_quantity=0
        for i in range(0,move_length):
            (price, level) = price_levels[i]
            orders=sorted(level.items(), key=lambda t: t[0])
            len_orders=len(orders)
            for j in range(0,len_orders):
                (orderid,order_quantity) = orders[j]
                # More than 1 tick move and last level and last order and last order.quantity > 1
                if match_case == 'P' and tick_move > 1 and i == move_length-1 and len_orders-1 == j and order_quantity > 1:
                    # Match half
                    match_order_quantity+=order_quantity/2
                    #trade_messages.append('{0:d},{1:.0f}'.format(order_quantity/2,match_price))
                    # Actually want trade to say what it executed at not what the match price was
                    trade_messages.append('{0},{1:d},{2:.0f}'.format(side,order_quantity/2,price))
                    modify_messages.append('{0:d},{1},{2:d},{3:.0f}'.format(orderid,side,order_quantity-order_quantity/2,price))
                else:
                    match_order_quantity+=order_quantity
                    #trade_messages.append('{0:d},{1:.0f}'.format(order_quantity,match_price))
                    # Actually want trade to say what it executed at not what the match price was
                    trade_messages.append('{0},{1:d},{2:.0f}'.format(side,order_quantity,price))
                    cancel_messages.append('{0:d},{1},{2:d},{3:.0f}'.format(orderid,side,order_quantity,price))

        return match_order_quantity
        
    def stabilize_order_book(self,side,match_price):
        """
        Recover the order book to a stable state as if it was responding to the market
        i.e filling in missing levels and balancing the boook levels
        """
        top_bid=self.ob.get_top_bid()
        top_ask=self.ob.get_top_ask()
        bottom_bid=self.ob.get_bottom_bid()
        bottom_ask=self.ob.get_bottom_ask()
        add_ticks=int((top_ask-top_bid)/self.tick_size)-1
        
        if side=='B':
            tick_direction=+1
            top_price=top_bid
            bottom_price=bottom_bid
            number_to_back_fill=5-len(self.ob.buyLevels.items())
        else:
            tick_direction=-1
            top_price=top_ask
            bottom_price=bottom_ask
            number_to_back_fill=5-len(self.ob.sellLevels.items())
            
        for i in range(1,add_ticks+1):
            self.print_add(self.gen_order_id(),flip_side(side),self.strategy.choose_quantity(),top_price+(tick_direction*i*self.tick_size))
        
        for i in range(1,number_to_back_fill+1):
            self.print_add(self.gen_order_id(),side,self.strategy.choose_quantity(),bottom_price+(-tick_direction*i*self.tick_size))
        
        self.cancel_order_book_trailing_levels()
        
    def cancel_order_book_trailing_levels(self):
        buy_price_levels=sorted(self.ob.buyLevels.items(), key=lambda t: t[0], reverse=True)
        sell_price_levels=sorted(self.ob.sellLevels.items(), key=lambda t: t[0], reverse=False)
        self.cancel_levels(buy_price_levels,'B')
        self.cancel_levels(sell_price_levels,'S')
                
    def cancel_levels(self,levels,side):
        len_levels=len(levels)
        for i in range(len_levels-1,4,-1):
            (price, level) = levels[i]
            for (order_id,quantity) in level.items():
                self.print_cancel(order_id,side,quantity,price)
    
    def allow_price_move(self,side):
        if side=='B' and self.ob.get_mid() >= self.bid_limit:
            return True
            
        if side=='S' and self.ob.get_mid() <= self.ask_limit:
            return True
        
        return False
            
    def gen_price_move_messages(self,order_id,side,quantity,price,trade_messages,cancel_messages,modify_messages):    
        match_message='{0:d},{1},{2:d},{3:.0f}'.format(order_id,side,quantity,price)
        # If we know the move and the side then create all the events to
        # make it happen in a stable and precise set of events
        self.ob.add(match_message)
        print('A,'+match_message)
        
        self.error_strategy.gen_bogus_add_order(side,quantity,price,self.gen_order_id,flip_side,self.ob)
        # Here we can generate a bogus add order which causes a 
        # match with no trades occuring error.
        #if self.errors and random.randint(1,500) == 1:
        #    error_order_id=self.gen_order_id()
        #    best_sell_le_best_buy_message='{0:d},{1},{2:d},{3:.0f}'.format(error_order_id,flip_side(side),1,price)
        #    self.ob.add(best_sell_le_best_buy_message)
        #    print('A,'+best_sell_le_best_buy_message)
        #    self.best_sell_le_best_buy_error += 1
            
        self.error_strategy.gen_bogus_trade(side,quantity,price,self.tick_size)
        # Here we can generate a bogus trade with no corresponding order
        #if self.errors and random.randint(1,500) == 1:
        #    no_order_for_trade_message='{0:d},{1:.0f}'.format(1,price-self.tick_size)
        #    print('T,'+no_order_for_trade_message)
        #    self.no_order_for_trade_error += 1
            
        for tm in trade_messages:
            self.ob.trade(tm)
            print('T,'+tm)
            
        self.ob.cancel(match_message)
        print('X,'+match_message)
        
        for cm in cancel_messages:
            self.ob.cancel(cm)
            print('X,'+cm)
            
        for mm in modify_messages:
            self.ob.modify(mm)
            print('M,'+mm)
    ################################################################################
         
    ################################################################################
    # Helper methods
    ################################################################################
    def get_price_orders(self,side,price):
        # Only add if we have space to add
        if side=='B':
            price_orders=self.ob.buyLevels[price]
        else:
            price_orders=self.ob.sellLevels[price]
            
        return price_orders
    
    def gen_order_id(self):
        self.last_order_id+=1
        return self.last_order_id          
    ################################################################################
   
################################################################################
# Helper functions
################################################################################
def flip_side(side):
    """
    Flip the side
    """
    if side=='B':
        return 'S'
    else:
        return 'B'
################################################################################
