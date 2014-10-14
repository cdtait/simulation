'''
Created on Oct 08, 2014

@author: ctait
'''

import random
import bisect
import operator

"""
These are the supporting strategies we can build on to simulate the
pricing, order and trade movements.Developing strategies may help
to develop predictive models. Many instances of Strategies possible 
100s could be mixed and applied at the same time.
"""
class SimulationStrategies:
    '''
    This is a basic random strategy
    '''
    class RandomStrategy(object):
        def __init__(self,order_book,max_order_quantity,tick_size):
            self.ob = order_book
            self.max_order_quantity=max_order_quantity
            self.tick_size=tick_size
            random.seed(113)
            # Modifications 79% Adds 10%  Cancels 10% and Trade movement %1 of time
            weighted_event_choices = [('A', 10), ('M', 79), ('X', 10), ('T', 1)]
            self.event_choices, event_weights = zip(*weighted_event_choices)
            self.eventcumdist = list(accumulate(event_weights))

        ################################################################################
        # Random choose function section
        ################################################################################
        def choose_event(self):
            event=self.event_choices[bisect.bisect(self.eventcumdist, 
                                                   random.random() * self.eventcumdist[-1])]
            return event
        
        def choose_tick_move(self):
            weighted_tick_choices = [(1,6),(2,3),(3,1)]
            tick_choices, tick_weights = zip(*weighted_tick_choices)
            tickcumdist=list(accumulate(tick_weights))
            tick_move=tick_choices[bisect.bisect(tickcumdist, random.random() * tickcumdist[-1])]
            return tick_move
        
        ################################################################################
        # Random choose function section
        ################################################################################
        def choose_price(self):
            side_choice=self.choose_side()
            if side_choice=='B':
                price_choices_buy=self.ob.buyLevels.keys()
                return (side_choice,random.choice(price_choices_buy))
            else:
                price_choices_sell=self.ob.sellLevels.keys()
                return (side_choice,random.choice(price_choices_sell))
            
        def choose_side(self):
            side_choices = ['B', 'S']
            return random.choice(side_choices)
    
        def choose_order(self,price_orders):
            return random.choice(price_orders.items())
            
        def choose_quantity(self,quantity=0):
            quantities=range(1,self.max_order_quantity+1)
            try:
                quantities.remove(quantity)
            except:
                pass
            return random.choice(quantities)
        
        def choose_replacement_price(self,price,side,price_level_prices):
            if side=='B':
                end_price=min(price_level_prices)-self.tick_size
            else:
                end_price=max(price_level_prices)+self.tick_size
                
            return random.choice([end_price]+[price,])
        
        def choose_match_case(self):
            match_case_choices = ['P', 'F']
            return random.choice(match_case_choices)
        ################################################################################
            
def accumulate(iterable, func=operator.add):
    'Return running totals'
    it = iter(iterable)
    total = next(it)
    yield total
    for element in it:
        total = func(total, element)
        yield total